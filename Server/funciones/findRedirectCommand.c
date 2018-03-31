//
// Created by tincho on 16/09/17.
//
/**
 * verifica si existen caracteres de redireccion en la cadena, si los hay obtiene los fileDesciptor
 * y los almacena en los previstos en el argumento
 * @param args cadena a analizar
 * @param fdin, fileDescriptor para redireccion de la entradaa
 * @param fdout, fileDescriptor para redireccion de la Salida
 * @return
 */
 void findRedirectCommand(char*  args,int* fdin,int* fdout){
     char *in=NULL;
     char *out=NULL;
    in=strstr(args,"<");
    out=strstr(args,">");
    if(in!=NULL) {
        if (*in == '<') {
            *(in+1)='\0';
            *in = '\0';
            in += 2;
            *fdin=open(in, O_RDONLY, 0600);
        }
    }
    if(out!=NULL) {
        if (*out == '>') {
            *(out-1)='\0';
            *out = '\0';
            out += 2;
            *fdout=open(out, O_WRONLY | O_CREAT, 0600);
        }
    }
}