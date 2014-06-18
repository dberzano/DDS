// Copyright 2014 GSI, Inc. All rights reserved.
//
// Log engine core.
//
#ifndef LOGGER_H
#define LOGGER_H

// BOOST
#define BOOST_LOG_DYN_LINK
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/current_process_id.hpp>
#include <boost/log/attributes/current_process_name.hpp>
#include <boost/log/attributes/current_thread_id.hpp>

// STD
#include <fstream>

// Main macro to be used for logging in DDS
// Example: LOG(trace) << "My message";
#define LOG(severity) BOOST_LOG_SEV(MiscCommon::Logger::instance().logger(), severity)

namespace MiscCommon
{
    //==========================
    // namespace logging = boost::log;
    // namespace src = boost::log::sources;
    // namespace expr = boost::log::expressions;
    // namespace sinks = boost::log::sinks;
    // namespace keywords = boost::log::keywords;

    // BOOST_LOG_SEV(lg, trace) << "A trace severity message";
    //    BOOST_LOG_SEV(lg, debug) << "A debug severity message";
    //    BOOST_LOG_SEV(lg, info) << "An informational severity message";
    //    BOOST_LOG_SEV(lg, warning) << "A warning severity message";
    //    BOOST_LOG_SEV(lg, error) << "An error severity message";
    //    BOOST_LOG_SEV(lg, fatal) << "A fatal severity message";

    // BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(gLogger, boost::log::sources::logger_mt)

    // BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)
    // BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", boost::log::trivial::severity_level)
    // BOOST_LOG_ATTRIBUTE_KEYWORD(process_id, "ProcessID", unsigned int)
    // BOOST_LOG_ATTRIBUTE_KEYWORD(thread_id, "ThreadID", unsigned int)

    class Logger
    {
      public:
        void init(const std::string& _fileName)
        {
            using namespace boost::log;

            // core::get()->reset_filter();
            // core::get()->remove_all_sinks();

            boost::log::formatter formatter = expressions::stream //<< expressions::attr<unsigned int>("LineID") << " "
                                              << expressions::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S <")
                                              << expressions::attr<int>("Severity") << "> "
                                              << expressions::attr<attributes::current_process_id::value_type>("ProcessID") << " "
                                              << expressions::attr<attributes::current_thread_id::value_type>("ThreadID") << " "
                                              << expressions::attr<std::string>("Process") << " " << expressions::smessage;

            // Logging to file
            boost::shared_ptr<sinks::synchronous_sink<sinks::text_file_backend>> fileSink = add_file_log(
                keywords::file_name = _fileName, keywords::open_mode = (std::ios::out | std::ios::app), keywords::rotation_size = 10 * 1024 * 1024);
            fileSink->set_formatter(formatter);
            fileSink->locked_backend()->auto_flush(true);

            // Logging to console
            boost::shared_ptr<sinks::synchronous_sink<sinks::text_ostream_backend>> consoleSink = add_console_log();
            // consoleSink->set_filter(expressions::attr<int>("Severity") == trivial::info || expressions::attr<int>("Severity") == trivial::error);
            // consoleSink->set_filter(trivial::severity == trivial::info);
            consoleSink->set_formatter(formatter);

            /*
                        typedef boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend> text_sink_t;
                        boost::shared_ptr<text_sink_t> sink = boost::make_shared<text_sink_t>();

                        sink->locked_backend()->add_stream(boost::make_shared<std::ofstream>(_fileName, std::ofstream::app));

                        boost::shared_ptr<std::ostream> stream(&std::clog, boost::empty_deleter());
                        sink->locked_backend()->add_stream(stream);

                        // sink = logging::add_console_log();

                        // sink->set_filter(expr::attr< int >("Severity") >= 3);

                        sink->locked_backend()->auto_flush(true);

                        sink->set_formatter(boost::log::expressions::stream
                                            << boost::log::expressions::attr<unsigned int>("LineID") << ": "
                                            << boost::log::expressions::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S <")
                                            << boost::log::trivial::severity << "> " << boost::log::expressions::smessage);

                        boost::log::core::get()->add_sink(sink);
             */

            add_common_attributes();
            // core::get()->add_global_attribute("ProcessID", attributes::current_process_id());
            core::get()->add_global_attribute("Process", attributes::current_process_name());
            // core::get()->add_global_attribute("ThreadID", attributes::current_thread_id());
        }

      public:
        // typedef boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level> logger_t;
        typedef boost::log::sources::logger_mt logger_t;

        static Logger& instance()
        {
            static Logger instance;
            return instance;
        }

        logger_t& logger()
        {
            return fLogger;
        }

      private:
        logger_t fLogger;
    };
};
#endif