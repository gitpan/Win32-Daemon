use Win32::Service::Daemon; 

%Hash = (
    name    =>  'PerlTest',
    display =>  'Oh my GOD, Perl is a service!',
    path    =>  'c:\perl\bin\perl.exe s:\perl\ext\win32\Daemon\test\daemon.pl',
    user    =>  '',
    pwd     =>  '',
);

if( Win32::Service::Daemon::CreateService( \%Hash ) )
{
    print "Successfully added.\n";
}
else
{
    print "Failed to add service: " . GetError() . "\n";
}



print "finished.\n";

sub DumpError
{
    print GetError(), "\n";
}

sub GetError
{
    return( Win32::FormatMessage( Win32::Service::Daemon::GetLastError() ) );
}
