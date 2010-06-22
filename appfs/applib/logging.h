/*

Header file for Logging.

This class provides a standardized way for sending error, warning
and informational messages back to the standard output.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                21th June 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

// NOTE: Unlike in the Python libraries, messages are not automatically
//       intended on newline characters.

#include <string>

class Logging
{
        public:
		static bool verbose;
		static void showErrorW(std::string msg);
		static void showErrorO(std::string msg);
		static void showWarningW(std::string msg);
		static void showWarningO(std::string msg);
		static void showInfoW(std::string msg);
		static void showInfoO(std::string msg);
		static void showSuccessW(std::string msg);
		static void showSuccessO(std::string msg);
		static void setApplicationName(std::string name);
}
