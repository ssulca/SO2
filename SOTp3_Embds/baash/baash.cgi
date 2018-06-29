#!/usr/bin/perl

use strict;
use CGI ':standard';
# La funcion Open3 de la libreria IPC permite el manejo de pipes
use IPC::Open3;
$ENV{PATH} = '/sbin:/usr/bin:/bin:/usr/sbin/'; # para la ejecucion del bash


print header,
      start_html('web terminal');

local (*IN,*OUT);

my $out = '';
my $cmd ;
my $pid = open3(\*IN, \*OUT ,\*OUT,'./rec/bash');

print h1("Baash");
$cmd = param("cmd"); # toma el comando enviado en el formulario

print IN "$cmd\n"; # comand to bash
print IN "exit\n"; # close bash

# leer pipe
$out = do{local $/;<OUT>};

#close BO; # cierra pipes
#close BI;
waitpid($pid,0);

$out =~ s/\n/<br>/g; # convierte "\n" en "<br>"

# formulario
print start_form;
print h2("Comando");
print textarea(-name=>'cmd',-id=>'cmd');
print submit('Áction','send');
print end_form;
print h2("Salida");
print p("$out"); # imprime salida del bash

print end_html;
