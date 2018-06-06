#!/usr/bin/perl

use strict;
use warnings FATAL => 'all';
use CGI ':standard';
use IPC::Open3;

####### def FILES ######
# OUTFILE :

####### end def ######

# definicion variables globales
my $load_print='';
my $remove_print='';

# si los campos no estan vacios se remueve o carga;
if (defined param('load_btn')) {
	load_module();
}
if(defined param('remove_btn')){
	remove_module();
} 

# cabecera html
print header,
      start_html('Modules_RPI');

print h1('Modulos');

print h2('Modululos Actuales');

# show system modules
print pre(`lsmod`);

# secciond de gestion de modulos
print h2('Upload & Load Module');

print "$load_print"; # mensajes de upload module
print "$remove_print"; # mensajes de remove module


print start_form; # Formulario
# seccion upload & load
print label('Load Module');
print submit('load_btn','Load Module');
print filefield('mod_file','startig value',80,80);
print '<br><br>';

# seccion para remover modulos
print label('Remove module');
print textfield(-name=>'rm_mod',-id=>'rm_mod',);
print submit('remove_btn','Remove Module');

print end_form;

print end_html(); # Final de pagina

# Carga el kernel
sub load_module{
	my $file_handler = upload('mod_file');  # Carga un handler al recibir archivo

    if(defined $file_handler){ # verifica si se subio un archivo (verif handler)
		my $filename = param('mod_file'); # get file name

		if($filename =~ m/\.ko$/){ # verifica el archivo
            my $bytesread;
            my $buffer;
            my $ls='';

            # save file
            my $handler = $file_handler->handle;

            open OUTFILE ,"> modules/$filename";
            while($bytesread = $handler->read ($buffer,1024)){
                print OUTFILE $buffer;
            }
            # carga el modulo en el kernel

            local (*IN,*OUT); # def pipes
            my $pidl = open3(\*IN, \*OUT ,0,'recursos/exec_insmod /var/www/cgi-bin/modules/$filename');

            $ls = do{local $/;<OUT>}; # leer el pipe
			$ls =~ s/\n/<br>/g; # convierte "\n" en "<br>"

            $load_print .= p('$ls'); # load mesage
            $load_print .= p('load succes') if ($ls eq '');

            close BO;
            close BI;
            waitpid($pidl, 0);

		}
        else{
			$load_print .= p('module file unrecognized');
		}
	}
    else{
		$load_print .= p('no hay modulos para cargar');
	}
}

# elimina el moduo de kernel
sub remove_module{
	my $rm_mod = param('rm_mod'); # load name module to remove!

    if(defined $rm_mod and ($rm_mod ne "")){ # verifica el campo
        my $ls = '';
        local (*IN,*OUT);
        my $pidr = open3(\*IN, \*OUT ,0 ,"recursos/exec_rmmod $rm_mod ");

        $ls = do{local $/;<OUT>}; # leer el pipe
        $ls =~ s/\n/<br>/g;

        $remove_print .= p("$ls");
        $remove_print .= p('modulo removido') if ($ls eq "");

        close BO;
        close BI;
        waitpid($pidr, 0);
	}
}
