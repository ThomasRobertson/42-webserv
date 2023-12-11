#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

void print(char *str)
{
	printf("%s\n", str);
}

void settingCookie(char *str)
{
	write(STDOUT_FILENO, "Set-Cookie: hello=", 18);
	write(STDOUT_FILENO, str, strlen(str));
	write(STDOUT_FILENO, "\n", 1);
}

int main(int ac, char **av, char **env)
{
	(void)ac;
	char *cookie;
	char *cookie_value;

	for (int i = 0; env[i]; i++)
	{
		if (strncmp("HTTP_COOKIE", env[i], 11) == 0)
		{
			if (env[i] + 12)
				cookie = strdup(env[i] + 12);
		}
	}
	char *token = strtok(cookie, "; ");
	while (token && strncmp("hello", token, 5) != 0)
	{
		token = strtok(NULL, "; ");
	}
	if (token && strncmp("hello", token, 5) == 0)
	{
		if ((token + 6))
			cookie_value = strdup(token + 6);
		else
			cookie_value = strdup("1");

		if (strlen(cookie_value) == 1  && *cookie_value < '9')
			*cookie_value += 1;
		else
		{
			free(cookie_value);
			cookie_value = strdup("1");
		}
	}
	else
		cookie_value = strdup("1");
	settingCookie(cookie_value);
	printf("Content-Type: text/plain");

	printf("\n\n");
	printf("       444444444   222222222222222                 CCCCCCCCCCCCC       GGGGGGGGGGGGGIIIIIIIIII\n");
	printf("      4::::::::4  2:::::::::::::::22            CCC::::::::::::C    GGG::::::::::::GI::::::::I\n");
	printf("     4:::::::::4  2::::::222222:::::2         CC:::::::::::::::C  GG:::::::::::::::GI::::::::I\n");
	printf("    4::::44::::4  2222222     2:::::2        C:::::CCCCCCCC::::C G:::::GGGGGGGG::::GII::::::II\n");
	printf("   4::::4 4::::4              2:::::2       C:::::C       CCCCCCG:::::G       GGGGGG  I::::I  \n");
	printf("  4::::4  4::::4              2:::::2      C:::::C             G:::::G                I::::I  \n");
	printf(" 4::::4   4::::4           2222::::2       C:::::C             G:::::G                I::::I  \n");
	printf("4::::444444::::444    22222::::::22        C:::::C             G:::::G    GGGGGGGGGG  I::::I  \n");
	printf("4::::::::::::::::4  22::::::::222          C:::::C             G:::::G    G::::::::G  I::::I  \n");
	printf("4444444444:::::444 2:::::22222             C:::::C             G:::::G    GGGGG::::G  I::::I  \n");
	printf("          4::::4  2:::::2                  C:::::C             G:::::G        G::::G  I::::I  \n");
	printf("          4::::4  2:::::2                   C:::::C       CCCCCCG:::::G       G::::G  I::::I  \n");
	printf("          4::::4  2:::::2       222222       C:::::CCCCCCCC::::C G:::::GGGGGGGG::::GII::::::II\n");
	printf("        44::::::442::::::2222222:::::2        CC:::::::::::::::C  GG:::::::::::::::GI::::::::I\n");
	printf("        4::::::::42::::::::::::::::::2          CCC::::::::::::C    GGG::::::GGG:::GI::::::::I\n");
	printf("        444444444422222222222222222222             CCCCCCCCCCCCC       GGGGGG   GGGGIIIIIIIIII\n");

	printf("\nHello ! You've been visiting this (awesome) website for the ");
	if (strcmp("1", cookie_value) == 0)
		printf("1st");
	else if (strcmp("2", cookie_value) == 0)
		printf("2nd");
	else if (strcmp("3", cookie_value) == 0)
		printf("3rd");
	else
		printf("%sth", cookie_value);
	printf(" time!\n");

	printf("\n\nDisplaying arguments :");
	for (int i = 0; av[i]; i++)
		print(av[i]);
	printf("\n\nDisplaying env :");
	for (int i = 0; env[i]; i++)
		print(env[i]);

	printf("\n\nDisplaying body send (if any) :\n");
	char ch[BUFFER_SIZE + 1];
	sleep(1);
	size_t len = 0;
	do
	{
		memset(ch, '\0', BUFFER_SIZE + 1);
		len = read(STDIN_FILENO, ch, BUFFER_SIZE);
		printf("%s", ch);
		//write(STDOUT_FILENO, ch, len);
		// printf(ch, len);
	} while (len > 0);

	printf("\n\nEND OF FILE\n");

	free(cookie);
	free(cookie_value);
}
