#pragma once

#include "session_manager_interface.hpp"

#include <client/simple_client.hpp>

#include <nscapi/nscapi_core_wrapper.hpp>

#include <RegexController.h>
#include <StreamResponse.h>

#include <string>

class metrics_controller : public Mongoose::RegexpController {
  boost::shared_ptr<session_manager_interface> session;
  const nscapi::core_wrapper* core;
  const unsigned int plugin_id;

  typedef std::vector<std::pair<std::string, std::string> > arg_vector;

 public:
  metrics_controller(const int version, const boost::shared_ptr<session_manager_interface>& session, const nscapi::core_wrapper* core, unsigned int plugin_id);

  void get_metrics(Mongoose::Request& request, boost::smatch& what, Mongoose::StreamResponse& response);
};
