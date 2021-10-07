/*
 * Copyright (C) 2019 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include <ignition/common/Console.hh>
#include <ignition/common/Profiler.hh>
#include <ignition/fuel_tools/Interface.hh>
#include <ignition/gui/Application.hh>
#include <ignition/gui/MainWindow.hh>

// Include all components so they have first-class support
#include "ignition/gazebo/components/components.hh"
#include "ignition/gazebo/Conversions.hh"
#include "ignition/gazebo/gui/GuiRunner.hh"
#include "ignition/gazebo/gui/GuiSystem.hh"

using namespace ignition;
using namespace gazebo;

/// \todo(anyone) Move to GuiRunner::Implementation when porting to v5
/// \brief Mutex to protect the plugin update.
static std::mutex gUpdateMutex;

/////////////////////////////////////////////////
GuiRunner::GuiRunner(const std::string &_worldName)
{
  this->setProperty("worldName", QString::fromStdString(_worldName));

  auto win = gui::App()->findChild<ignition::gui::MainWindow *>();
  auto winWorldNames = win->property("worldNames").toStringList();
  winWorldNames.append(QString::fromStdString(_worldName));
  win->setProperty("worldNames", winWorldNames);

  this->stateTopic = "/world/" + _worldName + "/state";

  common::addFindFileURICallback([] (common::URI _uri)
  {
    return fuel_tools::fetchResource(_uri.Str());
  });

  igndbg << "Requesting initial state from [" << this->stateTopic << "]..."
         << std::endl;

  this->RequestState();

  // Periodically update the plugins
  QPointer<QTimer> timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &GuiRunner::UpdatePlugins);
  timer->start(33);
}

/////////////////////////////////////////////////
GuiRunner::~GuiRunner() = default;

/////////////////////////////////////////////////
void GuiRunner::RequestState()
{
  // set up service for async state response callback
  std::string id = std::to_string(gui::App()->applicationPid());
  std::string reqSrv =
      this->node.Options().NameSpace() + "/" + id + "/state_async";
  this->node.Advertise(reqSrv, &GuiRunner::OnStateAsyncService, this);
  ignition::msgs::StringMsg req;
  req.set_data(reqSrv);

  // send async state request
  this->node.Request(this->stateTopic + "_async", req);
}

/////////////////////////////////////////////////
void GuiRunner::OnPluginAdded(const QString &)
{
  // This function used to call Update on the plugin, but that's no longer
  // necessary. The function is left here for ABI compatibility.
}

/////////////////////////////////////////////////
void GuiRunner::OnStateAsyncService(const msgs::SerializedStepMap &_res)
{
  this->OnState(_res);

  // todo(anyone) store reqSrv string in a member variable and use it here
  // and in RequestState()
  std::string id = std::to_string(gui::App()->applicationPid());
  std::string reqSrv =
      this->node.Options().NameSpace() + "/" + id + "/state_async";
  this->node.UnadvertiseSrv(reqSrv);

  // Only subscribe to periodic updates after receiving initial state
  if (this->node.SubscribedTopics().empty())
    this->node.Subscribe(this->stateTopic, &GuiRunner::OnState, this);
}

/////////////////////////////////////////////////
void GuiRunner::OnState(const msgs::SerializedStepMap &_msg)
{
  IGN_PROFILE_THREAD_NAME("GuiRunner::OnState");
  IGN_PROFILE("GuiRunner::Update");

  std::lock_guard<std::mutex> lock(gUpdateMutex);
  this->ecm.SetState(_msg.state());

  // Update all plugins
  this->updateInfo = convert<UpdateInfo>(_msg.stats());
  this->UpdatePlugins();
  this->ecm.ClearNewlyCreatedEntities();
  this->ecm.ProcessRemoveEntityRequests();
}

/////////////////////////////////////////////////
void GuiRunner::UpdatePlugins()
{
  auto plugins = gui::App()->findChildren<GuiSystem *>();
  for (auto plugin : plugins)
  {
    plugin->Update(this->updateInfo, this->ecm);
  }
}
