/*
 * Copyright (C) 2004-2016 Michael Medin
 *
 * This file is part of NSClient++ - https://nsclient.org
 *
 * NSClient++ is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * NSClient++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NSClient++.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "plugin_list.hpp"

namespace nsclient {
struct channels : public plugins_list_with_listener {
  channels(nsclient::logging::logger_instance logger) : plugins_list_with_listener(logger) {}

  void add_plugin(plugin_type plugin) {
    if (!plugin || !plugin->hasNotificationHandler()) return;
    plugins_list_with_listener::add_plugin(plugin);
  }
};
struct event_subscribers : public plugins_list_with_listener {
  event_subscribers(nsclient::logging::logger_instance logger) : plugins_list_with_listener(logger) {}

  void add_plugin(plugin_type plugin) {
    if (!plugin || !plugin->has_on_event()) return;
    plugins_list_with_listener::add_plugin(plugin);
  }
};
}  // namespace nsclient