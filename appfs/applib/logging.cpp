/*

Code file for Logging.

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

static class Logging
{
	public bool verbose = false;
	private std::string appname = "apptools";
	private std::string appblnk = "        ";

	public void showErrorW(std::string msg)
		showMsgW("error", msg);

	public void showErrorO(std::string msg)
		showMsgO(msg);

	public void showWarningW(std::string msg)
                showMsgW("warning", msg);

        public void showWarningO(std::string msg)
                showMsgO(msg);

	public void showInfoW(std::string msg)
	{
		if (verbose)
	                showMsgW("info", msg);
	}

        public void showInfoO(std::string msg)
	{
		if (verbose)
	                showMsgO(msg);
	}

	public void showSuccessW(std::string msg)
                showMsgW("success", msg);

        public void showSuccessO(std::string msg)
                showMsgO(msg);
	
	public void setApplicationName(std::string name)
	{
		appname = name;
		appblnk = "";
		while (appblnk.length() < appname.length())
			appblnk += " ";
	}

	// Internal functions.

	private void showMsgW(std::string type, std::string msg)
	{
		std::string resultMessage = "";
		resultMessage = buildPrefix("error");
		resultMessage += msg;
		printf("%s\n", resultMessage.c_str());
	}

	private void showMsgO(std::string msg)
	{
		std::string resultMessage = appblnk;
		resultMessage += "    ";    // ": [ "
		resultMessage += "       "; // "_______"
		resultMessage += "   ";     // " ] "
		resultMessage += msg;
		printf("%s\n", resultMessage.c_str());
	}

	private std::string alignText(std::string text, int len)
	{
		std::string resultMessage = text;
		while (resultMessage.length() < len)
			resultMessage += 1;
		return resultMessage;
	}

	private std::string buildPrefix(std::string type)
	{
		std::string resultMessage = "";
		resultMessage = appname;
		resultMessage += ": [ ";
		resultMessage += alignText(type, 7);
		resultMessage += " ] ";
		return resultMessage;
	}
}
