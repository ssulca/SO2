#!/usr/bin/perl
# fuente https://gist.github.com/aras-p/47e2252d6b1fa57d3619fd8e021690ec
use warnings;
use strict;
use List::Util 'first';

my $cpu = 'Unknown';
my $cpus = 0;
my $ram = 0;
my $datestring = '';
my $uptime = '';

open my $h, "/proc/cpuinfo";
if ($h)
{
    my @info = <$h>;
    close $h;
    $cpus = scalar(map /^processor/, @info);
    my $strCPU = first { /^model name/ } @info;
    $cpu = $1 if ($strCPU && $strCPU =~ /:\s+(.*)/);
}
open $h, "/proc/meminfo";
if ($h)
{
    my @info = <$h>;
    close $h;
    my $strRAM = first { /^MemTotal/ } @info;
    $ram = $1 * 1024 if ($strRAM && $strRAM =~ /:\s+(\d+)/);
}
open $h, "/proc/uptime";
{
    my @info = <$h>;
    close $h;
    @info = split(/ /,$info[0]);
    $uptime = $info[0];
}

$ram = int($ram / 1024 / 1024 / 1024 + 0.5);
$datestring = localtime();
print "CPU: '$cpu'\n";
print "CPU nucleos: $cpus\n";
print "RAM: $ram GB\n";
print "DATE: $datestring\n";
print "UPTIME: $uptime segundos \n";

print <<EOF;
<html><head><title>Recursos</title></head>

<body bgcolor=white>
<h1>Informacion de los recursos</h1>

<h2>CPU: </h2>
<FONT FACE="courier new"> $cpu </FONT>

<h2>CPU nucleos: </h2>
<FONT FACE="courier new"> $cpus </FONT>

<h2>RAM: </h2>
<FONT FACE="courier new"> $ram BG </FONT>

<h2>DATE: </h2>
<FONT FACE="courier new"> $datestring </FONT>

<h2>UPTIME: </h2>
<FONT FACE="courier new"> $uptime segundos </FONT>

</body></html>

EOF

