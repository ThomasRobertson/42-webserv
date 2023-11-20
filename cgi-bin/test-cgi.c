#include <stdio.h>
#include <errno.h>
#include <string.h>

#define BUFFER_SIZE 4096

int main(int ac, char **av, char **env)
{
	(void)ac;                                                                               
	printf("       444444444   222222222222222                 CCCCCCCCCCCCC       GGGGGGGGGGGGGIIIIIIIIII\n"),
	printf("      4::::::::4  2:::::::::::::::22            CCC::::::::::::C    GGG::::::::::::GI::::::::I\n"),
	printf("     4:::::::::4  2::::::222222:::::2         CC:::::::::::::::C  GG:::::::::::::::GI::::::::I\n"),
	printf("    4::::44::::4  2222222     2:::::2        C:::::CCCCCCCC::::C G:::::GGGGGGGG::::GII::::::II\n"),
	printf("   4::::4 4::::4              2:::::2       C:::::C       CCCCCCG:::::G       GGGGGG  I::::I  \n"),
	printf("  4::::4  4::::4              2:::::2      C:::::C             G:::::G                I::::I  \n"),
	printf(" 4::::4   4::::4           2222::::2       C:::::C             G:::::G                I::::I  \n"),
	printf("4::::444444::::444    22222::::::22        C:::::C             G:::::G    GGGGGGGGGG  I::::I  \n"),
	printf("4::::::::::::::::4  22::::::::222          C:::::C             G:::::G    G::::::::G  I::::I  \n"),
	printf("4444444444:::::444 2:::::22222             C:::::C             G:::::G    GGGGG::::G  I::::I  \n"),
	printf("          4::::4  2:::::2                  C:::::C             G:::::G        G::::G  I::::I  \n"),
	printf("          4::::4  2:::::2                   C:::::C       CCCCCCG:::::G       G::::G  I::::I  \n"),
	printf("          4::::4  2:::::2       222222       C:::::CCCCCCCC::::C G:::::GGGGGGGG::::GII::::::II\n"),
	printf("        44::::::442::::::2222222:::::2        CC:::::::::::::::C  GG:::::::::::::::GI::::::::I\n"),
	printf("        4::::::::42::::::::::::::::::2          CCC::::::::::::C    GGG::::::GGG:::GI::::::::I\n"),
	printf("        444444444422222222222222222222             CCCCCCCCCCCCC       GGGGGG   GGGGIIIIIIIIII\n"),

	printf("\n\nDisplaying arguments :\n");
	for (int i = 0; av[i]; i++)
		printf("%s\n", av[i]);
	printf("\n\nDisplaying env :\n");
	for (int i = 0; env[i]; i++)
		printf("%s\n", env[i]);

	printf("\n\nDisplaying content of file :\n");
	FILE* file = fopen(av[1], "r");
	char buffer[BUFFER_SIZE];
	if (file == NULL)
	{
		printf("Could not read the file : %s\n", strerror(errno));
		return 0;
	}
	int read = 0;
	while (read = fread(&buffer, BUFFER_SIZE, sizeof(char), file), read) {
		printf("%.*s", BUFFER_SIZE, buffer);
	}
}