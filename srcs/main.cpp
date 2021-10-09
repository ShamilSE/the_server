#include "../includes/WebServ.hpp"

int	main(int ac, char **av)
{
	if (ac > 2)
		std::cerr << "too many args" << std::endl;
	else
	{
		signal(SIGPIPE, SIG_IGN); /*	сигнал записи в закрытое соединение	*/
		std::string conf = (ac == 1) ? "configs/default.conf" : av[1];
		try
		{
			WebServ		WebServ(conf);
			WebServ.mainCycly();
		}
		catch (const std::string &error)
		{
			std::cerr << error << std::endl;
			return 1;
		}
	}
	return 0;
}
