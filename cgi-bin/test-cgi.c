#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

void putstr(char *str)
{
	int len = 0;
	while (str[len])
		len++;
	write(STDOUT_FILENO, str, len);
	write(STDOUT_FILENO, "\n", 1);
}

int main(int ac, char **av, char **env)
{
	(void)ac;
	
	putstr("       444444444   222222222222222                 CCCCCCCCCCCCC       GGGGGGGGGGGGGIIIIIIIIII"),
	putstr("      4::::::::4  2:::::::::::::::22            CCC::::::::::::C    GGG::::::::::::GI::::::::I"),
	putstr("     4:::::::::4  2::::::222222:::::2         CC:::::::::::::::C  GG:::::::::::::::GI::::::::I"),
	putstr("    4::::44::::4  2222222     2:::::2        C:::::CCCCCCCC::::C G:::::GGGGGGGG::::GII::::::II"),
	putstr("   4::::4 4::::4              2:::::2       C:::::C       CCCCCCG:::::G       GGGGGG  I::::I  "),
	putstr("  4::::4  4::::4              2:::::2      C:::::C             G:::::G                I::::I  "),
	putstr(" 4::::4   4::::4           2222::::2       C:::::C             G:::::G                I::::I  "),
	putstr("4::::444444::::444    22222::::::22        C:::::C             G:::::G    GGGGGGGGGG  I::::I  "),
	putstr("4::::::::::::::::4  22::::::::222          C:::::C             G:::::G    G::::::::G  I::::I  "),
	putstr("4444444444:::::444 2:::::22222             C:::::C             G:::::G    GGGGG::::G  I::::I  "),
	putstr("          4::::4  2:::::2                  C:::::C             G:::::G        G::::G  I::::I  "),
	putstr("          4::::4  2:::::2                   C:::::C       CCCCCCG:::::G       G::::G  I::::I  "),
	putstr("          4::::4  2:::::2       222222       C:::::CCCCCCCC::::C G:::::GGGGGGGG::::GII::::::II"),
	putstr("        44::::::442::::::2222222:::::2        CC:::::::::::::::C  GG:::::::::::::::GI::::::::I"),
	putstr("        4::::::::42::::::::::::::::::2          CCC::::::::::::C    GGG::::::GGG:::GI::::::::I"),
	putstr("        444444444422222222222222222222             CCCCCCCCCCCCC       GGGGGG   GGGGIIIIIIIIII"),

	putstr("\n\nDisplaying arguments :");
	for (int i = 0; av[i]; i++)
		putstr(av[i]);
	putstr("\n\nDisplaying env :");
	for (int i = 0; env[i]; i++)
		putstr(env[i]);

	putstr("\n\nDisplaying content of file is :");
	char ch[BUFFER_SIZE];
	sleep(1);
	size_t len = 0;
	do
	{
		len = read(STDIN_FILENO, ch, BUFFER_SIZE);
		write(STDIN_FILENO, ch, len);
		// putstr(ch, len);
	} while (len > 0);

	putstr("\n\nEND OF FILE\n");
}
