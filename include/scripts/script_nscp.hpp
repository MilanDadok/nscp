#pragma once

#include <scripts/script_interface.hpp>
#include <nscapi/nscapi_core_wrapper.hpp>
#include <nscapi/nscapi_settings_proxy.hpp>

namespace scripts {
	namespace nscp {

		struct tags {
			const static std::string simple_query_tag;
			const static std::string simple_exec_tag;
			const static std::string simple_submit_tag;
			const static std::string query_tag;
			const static std::string exec_tag;
			const static std::string submit_tag;
		};

		struct settings_provider_impl : public settings_provider {
			int plugin_id;
			nscapi::core_wrapper* core_;
			nscapi::settings_proxy settings_;
			settings_provider_impl(int plugin_id, nscapi::core_wrapper* core)
				: plugin_id(plugin_id)
				, core_(core)
				, settings_(plugin_id, core)
			{}

			virtual std::list<std::string> get_section(std::string section);
			virtual std::string get_string(std::string path, std::string key, std::string value);
			virtual void set_string(std::string path, std::string key, std::string value);
			virtual bool get_bool(std::string path, std::string key, bool value);
			virtual void set_bool(std::string path, std::string key, bool value);
			virtual int get_int(std::string path, std::string key, int value);
			virtual void set_int(std::string path, std::string key, int value);
			virtual void register_path(std::string path, std::string title, std::string description, bool advanced);
			virtual void register_key(std::string path, std::string key, std::string type, std::string title, std::string description, std::string defaultValue);
			virtual void save();

		};

		struct core_provider_impl : public core_provider {
			nscapi::core_wrapper* core_;
			core_provider_impl(nscapi::core_wrapper* core) : core_(core) {}

			virtual bool submit_simple_message(const std::string channel, const std::string command, const NSCAPI::nagiosReturn code, const std::string & message, const std::string & perf, std::string & response);
			virtual NSCAPI::nagiosReturn simple_query(const std::string &command, const std::list<std::string> & argument, std::string & msg, std::string & perf);
			virtual NSCAPI::nagiosReturn exec_simple_command(const std::string target, const std::string command, const std::list<std::string> &argument, std::list<std::string> & result);
			virtual NSCAPI::nagiosReturn exec_command(const std::string target, const std::string &request, std::string &response);
			virtual NSCAPI::nagiosReturn query(const std::string &request, std::string &response);
			virtual NSCAPI::nagiosReturn submit(const std::string target, const std::string &request, std::string &response);
			virtual NSCAPI::nagiosReturn reload(const std::string module);
			virtual void log(NSCAPI::log_level::level, const std::string file, int line, const std::string message);

		};
		struct nscp_runtime_impl : public nscp_runtime_interface {
			int plugin_id;
			nscapi::core_wrapper* core_;
			boost::shared_ptr<settings_provider_impl> settings_;
			boost::shared_ptr<core_provider_impl> core_provider_;

			nscp_runtime_impl(int plugin_id, nscapi::core_wrapper* core) 
				: plugin_id(plugin_id)
				, core_(core)
				, settings_(new settings_provider_impl(plugin_id, core)) 
				, core_provider_(new core_provider_impl(core))
			{}

			virtual void register_command(const std::string type, const std::string &command, const std::string &description);

			virtual boost::shared_ptr<settings_provider> get_settings_provider() {
				return settings_;
			}
			virtual boost::shared_ptr<core_provider> get_core_provider() {
				return core_provider_;
			}

		};
	}
}
