#pragma once

#include <string>
#include <exception>
#include <sstream>
#include <fstream>
#include <ctime>
#include <cassert>
#include <iomanip>

namespace solo {

	template<typename level>
	class log ;
	
	template<> // this partial version only implement some type support
	class log<void> {
	public:
		struct flush{} ;
		typedef flush flush_type ;
	} ;

	extern const log<void>::flush_type & flushlog ;

	template<>
	class log<log<void> > {

	protected:
		::std::stringstream _buffer ;
		::std::ofstream _outfile ;

	public:

		static inline void localtime(time_t &lt, tm& tstr) {
#if SOLO_PLATFORM == SOLO_PLATFORM_LINUX
			localtime_r(&lt, &tstr);
#else
			localtime_s(&tstr, &lt);
#endif
		}

		typedef struct _stream {
			log & _logger ;
			bool _need_log ;
			::std::stringstream _ss ;
			inline _stream(log & logger_, bool need_log_=true):_logger(logger_),_need_log(need_log_), _ss() {} ;
			inline _stream(const _stream & other):_logger(other._logger),_need_log(other._need_log),_ss() {} ;
			inline ~_stream(void) {
				if (!_need_log) {
					return ;
				}
				time_t  lt; 
				time(&lt);
				tm tstr ;
				log::localtime(lt, tstr) ;
				_logger 
					<< "["
					<< tstr.tm_year + 1900
					<< '-' << std::setw(2) << std::setfill('0') << tstr.tm_mon + 1 
					<< '-' << std::setw(2) << std::setfill('0') << tstr.tm_mday
					<< ' ' << std::setw(2) << std::setfill('0') << tstr.tm_hour 
					<< ':' << std::setw(2) << std::setfill('0') << tstr.tm_min 
					<< ':' << std::setw(2) << std::setfill('0') << tstr.tm_sec 
					<< ']' << ' ' ;
				_logger << _ss.str() << '\n' << flushlog ;
			}

			template<typename data_type>
				inline _stream & operator << (const data_type & data) {
					if (_need_log) {
						_ss << data ;
					}
					return *this ;
				}
		} stream_type ;

	private:
		inline void do_flush(void) {
			if (_buffer.tellp() > 0) {
				flush_impl() ;
			}
		}

	protected:
		virtual inline void flush_impl(void) {
			if (!_outfile.is_open()) {
				return ;
			}
			_outfile << _buffer.str() ;
			_buffer.str("") ;
			return ;
		}

		inline std::stringstream & buffer(void) {
			return _buffer ;
		}

		inline std::ofstream & file_stream(void) {
			assert(_outfile.is_open()) ;
			return _outfile ;
		}

		inline std::ofstream::pos_type tellp(void) {
			assert(_outfile.is_open()) ;
			return _outfile.tellp() ;
		}

	public:
		inline log(void) {
			_buffer.str("") ;
		}
		virtual inline ~log(void) {
			this->close() ;
		}

		inline bool open(::std::string filename) {
			if (_outfile.is_open()) {
				this->close() ;
			}
			_outfile.open(filename.c_str(), ::std::ios::app) ;
			if (!_outfile) {
				return false;
			}
			return true ;
		}

		inline void close(void) {
			this->do_flush() ;
			if (_outfile.is_open()) {
				_outfile.close() ;
			}
		}

		template<typename data_type>
			inline log & operator << (const data_type & data) {
				_buffer << data ;
				return *this ;
			}

		inline log & operator << (const log<void>::flush_type &) {
			this->do_flush() ;
			return *this ;
		}

		inline _stream stream(void) {
			return _stream(*this) ;
		}
	} ;

	typedef log<log<void> > verbose_log ;
}

