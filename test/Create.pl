use Win32::Daemon; 

%Hash = (
    name    =>  'PerlTest',
    display =>  'Oh my GOD, Perl is a service!',
    path    =>  'c:\perl\bin\perl.exe',
    user    =>  '',
    pwd     =>  '',
    parameters =>'c:\perl\scripts\daemon.pl',
);

if( Win32::Daemon::CreateService( \%Hash ) )
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
    return( Win32::FormatMessage( Win32::Daemon::GetLastError() ) );
}
