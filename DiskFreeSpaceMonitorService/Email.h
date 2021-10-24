#include<string>
class Email
{
private:
	std::string server;
	int port=465;
	std::string login;
	std::string mail_to;
	std::string password;
	std::string subject = "Disk Free Space Monitor Service";

public:
	Email(std::string s,int p, std::string from, std::string to, std::string pas):server(s),port(p),login(from),mail_to(to),password(pas) { };
	int sendEmail(std::string);

};