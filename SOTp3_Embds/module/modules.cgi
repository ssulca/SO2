#!/usr/bin/perl

use strict;
use warnings;
use CGI ':standard';
use IPC::Open3;

####### def FILES ######
# OUTFILE :
####### end def ######

# definicion variables globales
my $load_print='';
my $remove_print='';
my $filename = '';

# si los campos no estan vacios se remueve o carga;
if (defined param('load_btn'))
{
    upload_load();
}

if(defined param('remove_btn'))
{
    remove();
}

print header,
    start_html('Modules_RPI');

print h1('Modulos');

print h2('Modululos Actuales');

print pre(`lsmod`);

# gestion de modulos
print h2('Upload & Load Module');



print start_form; # Formulario
# seccion upload & load
print h3('Load Module');
print label('Load Module');
print submit('load_btn','Load Module');

print "$load_print"; # mensajes de upload module

print h3('Upload Module');

# fuente http://perldoc.perl.org/CGI.html
print filefield(-name=>'mod_file',
                -default=>'startig value',
                -size=>50,
                -maxlength=>80);

# seccion para remover modulos
print h3('Remove Module');
print label('Remove module');
print textfield(-name=>'rm_mod', -id=>'rm_mod',);
print submit('remove_btn','Remove Module');
print "$remove_print"; # mensajes de remove module

print end_form;

print end_html(); # Final de pagina

# Carga el kernel

sub upload_load
{
    my $file_handler = upload('mod_file');  # Carga un handler al recibir archivo
    my $ls;
    # undef may be returned if it's not a valid file handle
    if(defined $file_handler) # verifica si se subio un archivo (verif handler)
    {
        $filename = param('mod_file'); # get file name

		if($filename =~ m/\.ko$/) # verifica archivo .ko
        {
            my $bytesread;
            my $buffer;

            # Upgrade the handle to one compatible with IO::Handle:
            my $handler = $file_handler->handle;

            open (OUTFILE ,'>>','./modules/$filename');
            while($bytesread = $handler->read ($buffer,1024)) # save file
            {
                print OUTFILE $buffer;
            }
            # carga el modulo en el kernel

            local (*IN,*OUT); # def pipes
            my $pidl = open3(\*IN, \*OUT, \*OUT, "./recursos/exec_insmod /var/www/cgi-bin/modules/$filename");
            $ls = '';
            $ls .= do{local $/;<OUT>}; # leer el pipe
            $ls =~ s/\n/<br>/g; # convierte "\n" en "<br>"

            $load_print .= p("$ls"); # load mesage
            $load_print .= p('load succes') if ($ls eq '');

            close BO;
            close BI;
            waitpid($pidl, 0);

        }
        else
        {
            $load_print .= p("module file unrecognized $filename");
        }
    }
    else
    {
        $load_print .= p('no hay modulos para cargar');
    }
}

# elimina el moduo de kernel
sub remove
{
    my $rm_mod = param('rm_mod'); # load name module to remove!

    if(defined $rm_mod and ($rm_mod ne "")) # verifica el campo
    {
        my $ls = '';
        local (*IN,*OUT);
        my $pidr = open3(\*IN, \*OUT ,\*OUT ,"./recursos/exec_rmmod $rm_mod");

        $ls = do{local $/;<OUT>}; # leer el pipe
        $ls =~ s/\n/<br>/g;

        $remove_print .= p("$ls");
        $remove_print .= p('modulo removido') if ($ls eq "");

        close BO;
        close BI;
        waitpid($pidr, 0);
    }
}
