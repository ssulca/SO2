//
// Created by tincho on 08/09/17.
//

int buscarConPATH(char* path ,char* archivo);
/**
 * busca el archivo y copia la ruta
 * @param archivo
 * @param path
 * @return 0 si se encontro el archivo, -1 si el archivo o la ruta no existen
 */
int buscarArchivo(char* archivo,char*path){

    if(strstr( archivo,"~/" )!='\0') {
        strcpy(path,bash_cdHome(strstr( archivo,"~/" )+1));
    }else if(strstr( archivo,"../" )!='\0'){

        getcwd(path,1000); //recora el directorio actual para llegar al directorio padre

        char* aux=path;
        char* ultimo;
        while(*aux!='\0'){
            if(*aux=='/'){ultimo=aux;}
            aux+=1;
        }
        *ultimo='\0';

        strcat(path,archivo+2);
    }else if(strstr( archivo,"./" )!='\0'){
        strcpy(path,getcwd(NULL,0));
        strcat(path,archivo+1);

    }else if((strstr(archivo,"/"))==archivo){
        strcpy(path,archivo);
    }else{
        return buscarConPATH(path,archivo);

    }


    if(access(path,F_OK)!=0){
        *path='\0';
        return -1;
    }

    return 0;
}
/**
 * busca el archivo en las rutas por defeco en la variable PATH
 * @param path
 * @return
 */
int buscarConPATH(char* path,char* archivo){
    char PATH[1000];
    strcpy(PATH,getenv("PATH"));
    char*aux=strtok(PATH,":");
    while(aux!=NULL){
        strcpy(path,aux);
        strcat(path,"/");
        strcat(path,archivo);
        if(access(path,F_OK)==0){return 0;}
        aux=strtok(NULL,":");
    }

    *path='\0';
    return-1;

}