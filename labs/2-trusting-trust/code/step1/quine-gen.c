// convert the contents of stdin to their ASCII values (e.g., 
// '\n' = 10) and spit out the <prog> array used in Figure 1 in
// Thompson's paper.
#include <stdio.h>
#include <unistd.h>

int main(void) { 
    // put your code here.
    char buf[10000];
    ssize_t num_read = read(STDIN_FILENO, buf, 10000);
    
    printf("char prog[] = {\n");
	for(int i = 0; i < num_read; i++)
		printf("\t%d,%c", buf[i], (i+1)%8==0 ? '\n' : ' ');
	printf("0 };\n");
	printf("%s", buf);

	return 0;
}
