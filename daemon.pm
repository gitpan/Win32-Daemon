#  Daemon.pm
#  Header for the Win32::Daemon Perl extension.
#  (c) Dave Roth
#  Courtesy of Roth Consulting
#  http://www.roth.net/

package Win32::Daemon;

$PACKAGE = $Package = "Win32::Daemon";

$VERSION = 20000319;
require Exporter;
require DynaLoader;


@ISA= qw( Exporter DynaLoader );
    # Items to export into callers namespace by default. Note: do not export
    # names by default without a very good reason. Use EXPORT_OK instead.
    # Do not simply export all your public functions/methods/constants.
@EXPORT = qw(
    SERVICE_STOPPED
    SERVICE_RUNNING
    SERVICE_PAUSED
    SERVICE_START_PENDING
    SERVICE_STOP_PENDING
    SERVICE_CONTINUE_PENDING
    SERVICE_PAUSE_PENDING

    SERVICE_CONTROL_SHUTDOWN
    SERVICE_CONTROL_PARAMCHANGE
    SERVICE_CONTROL_NETBINDADD
    SERVICE_CONTROL_NETBINDADD
    SERVICE_CONTROL_NETBINDENABLE
    SERVICE_CONTROL_NETBINDDISABLE

    SERVICE_CONTROL_USER_DEFINED

    USER_SERVICE_BITS_1
    USER_SERVICE_BITS_2
    USER_SERVICE_BITS_3
    USER_SERVICE_BITS_4
    USER_SERVICE_BITS_5
    USER_SERVICE_BITS_6
    USER_SERVICE_BITS_7
    USER_SERVICE_BITS_8
    USER_SERVICE_BITS_9
    USER_SERVICE_BITS_10
);

@EXPORT_OK = qw(
);	

sub new{
    my( $type, @Options ) = @_;
    my $self = bless {};

    $Result = Win32::Perms::New( $self, @Options );
    if( ! $Result )
    {
		undef %self;
        return undef;
    }
    return( $self );
}

bootstrap $Package;

sub AUTOLOAD 
{
    # This AUTOLOAD is used to 'autoload' constants from the constant()
    # XS function.  If a constant is not found then control is passed
    # to the AUTOLOAD in AutoLoader.

    my( $Constant ) = $AUTOLOAD;
    my( $Result, $Value );
    $Constant =~ s/.*:://;

    $Result = Constant( $Constant, $Value );

    if( 0 == $Result )
    {
        # The extension could not resolve the constant...
        $AutoLoader::AUTOLOAD = $AUTOLOAD;
	    goto &AutoLoader::AUTOLOAD;
        return;
    }
    elsif( 1 == $Result )
    {
        # $Result == 1 if the constant is valid but not defined
        # that is, the extension knows that the constant exists but for
        # some wild reason it was not compiled with it.
        $pack = 0; 
        ($pack,$file,$line) = caller;
        print "Your vendor has not defined $Package macro $constname, used in $file at line $line.";
    }
    elsif( 2 == $Result )
    {
        # If $Result == 2 then we have a string value
        $Value = "'$Value'";
    }
        # If $Result == 3 then we have a numeric value

    eval "sub $AUTOLOAD { return( $Value ); }";
    goto &$AUTOLOAD;
}


END
{
    Win32::Daemon::StopService();
}


1;

__END__

=head1 NAME

Win32::Daemon - Extension enabling Win32 Perl scripts to be a service


=head1 SYNOPSIS

	use Win32::Daemon;
    Win32::Daemon::StartService();
        I<...process Perl code...>
    Win32::Daemon::StopService();

=head1 DESCRIPTION

This extension enables a Win32 Perl script to act as a true Win32 service.

=head1 FUNCTIONS

    CreateService()
    DeleteService() 
    GetLastError()
    GetServiceHandle()
    HideService()
    QueryLastMessage()
    RestoreService()
    SetServiceBits()
    ShowService()
    StartService()
    State()
    StopService()   
    Timeout()
    
=over 4

=item StartService()

This starts a new service thread. The script should call this as soon as possible.  When
the service manager starts the service Perl is started and the script is loaded.

=item StopService()

This will instruct the service to terminate.


=item QueryLastMessage()

This function returns the last message that the service manager has sent to the service.

Occasionally the service manager will send messages to the service. These messages 
typically request the service to change from one state to another.  It is important that
the Perl script responds to each message otherwise the service manager becomes confused
about the current state of the service. For example, if the service manager is submits
a C<SERVICE_PAUSE_PENDING> then it expects the Perl script to recognize the change to a paused
state and submit the new state by calling C<State( SERVICE_PAUSED )>.

You can update the service manager with the current status using the C<State()> function.

Possible values returned are:

    SERVICE_STOPPED............The service is stopped
    SERVICE_RUNNING............The service is running
    SERVICE_PAUSED.............The service is paused
    SERVICE_START_PENDING......The service manager is attempting to start the service
    SERVICE_STOP_PENDING.......The service manager is attempting to stop the service
    SERVICE_CONTINUE_PENDING...The service manager is attempting to resume the service
    SERVICE_PAUSE_PENDING......The service manager is attempting to pause the service
    SERVICE_INTERROGATE........The service manager is querying the service's state

    Windows 2000 specific messages:
    SERVICE_CONTROL_SHUTDOWN..........The machine is shutting down. This indicates that
                                      the service has roughly 20 seconds to clean up
                                      and terminate. This time can be extended by
                                      submitting SERVICE_STOP_PENDING via the State() function.
    SERVICE_CONTROL_PARAMCHANGE.......Service parameters have been modified.
    SERVICE_CONTROL_NETBINDADD........A network binding as been added.
    SERVICE_CONTROL_NETBINDREMOVE.....A network binding has been removed.
    SERVICE_CONTROL_NETBINDENABLE.....A network binding has been enabled.
    SERVICE_CONTROL_NETBINDDISABLE....A network binding has been disabled.

    SERVICE_CONTROL_USER_DEFINED......This is a user defined control. There are 127 of these
                                      beginning with SERVICE_CONTROL_USER_DEFINED as the base.


=item State([$NewState])

This function returns the current state of the service.  It can optionally update the
status of the service as well.  This is the last status reported to the service manager.

Optionally you can pass in a value that will be sent to the service manager.

Possible values returned (or submitted):

    SERVICE_STOPPED............The service is stopped
    SERVICE_RUNNING............The service is running
    SERVICE_PAUSED.............The service is paused
    SERVICE_START_PENDING......The service manager is attempting to start the service
    SERVICE_STOP_PENDING.......The service manager is attempting to stop the service
    SERVICE_CONTINUE_PENDING...The service manager is attempting to resume the service
    SERVICE_PAUSE_PENDING......The service manager is attempting to pause the service


=back

=head2 Example: Simple Service

This example service will delete all .tmp files from the c:\temp directory every
time it starts.  It will immediately terminate.

	use Win32::Daemon;

    # Tell the OS to start processing the service...
    Win32::Daemon::StartService();

    # Wait until the service manager is ready for us to continue...
    while( SERVICE_START_PENDING != Win32::Daemon::State() )
    {
        sleep( 1 );
    }

    # Now let the service manager know that we are running...
    Win32::Daemon::State( SERVICE_RUNNING );

    # Okay, go ahead and process stuff...
    unlink( glob( "c:\\temp\\*.tmp" ) );

    # Tell the OS that the service is terminating...
    Win32::Daemon::StopService();

This particular example does not really illustrate the capabilities of a Perl based service.

=head2 Example: Monitoring a directory






=head2 Example: Install the service

Assuming that Perl.exe is in c:\perl\bin and the service script is 
c:\perl\scripts\service.pl then this script will install the script
as a service.  Since no user is specified it defaults to the LocalSystem.

    use Win32::Daemon; 
    %Hash = (
        name    =>  'PerlTest',
        display =>  'Oh my GOD, Perl is a service!',
        path    =>  'c:\perl\bin\perl.exe',
        user    =>  '',
        pwd     =>  '',
        parameters =>'c:\perl\scripts\service.pl',
    );
    if( Win32::Daemon::CreateService( \%Hash ) )
    {
        print "Successfully added.\n";
    }
    else
    {
        print "Failed to add service: " . Win32::FormatMessage( Win32::Daemon::GetLastError() ) . "\n";
    }

