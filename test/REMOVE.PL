use Win32::Daemon; 

%Hash = (
    name    =>  'PerlTest',
    display =>  'Oh my GOD, Perl is a service!',
    path    =>  'c:\perl\bin\perl.exe',
    user    =>  '',
    pwd     =>  '',
    parameter =>'c:\source\perl\ext\win32\Daemon\test\daemon.pl',
);

if( Win32::Daemon::DeleteService( $Hash{name} ) )
{
    print "Successfully removed.\n";
}
else
{
    print "Failed to remove service: " . GetError() . "\n";
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
