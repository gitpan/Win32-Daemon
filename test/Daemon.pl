# Demonstration AutoStart Service using the Win32::Daemon
# Perl extension.
#

use Win32::Daemon;
use Win32::Process;
use Win32::Console;

my %List;
my ( $DB_DIR ) = ( $0 =~ /^(.*)\\[^\\]*$/ );
my $DB_FILE = "$DB_DIR/daemon.ini";
my $DB_LOG = "$DB_DIR/daemon.pid";
my $SLEEP_TIMEOUT = 1;
my $SERVICE_BITS = USER_SERVICE_BITS_8;
my $iTotalCount = ReadDB( $DB_FILE, \%List );

# Define how long to wait before a default status update is set.
Win32::Daemon::Timeout( 5 );

Win32::Daemon::StartService();
Win32::Daemon::SetServiceBits( $SERVICE_BITS );
Win32::Daemon::ShowService();

$Buffer = new Win32::Console();
$Buffer->Display();
$Buffer->Title( "Perl based AutoStart service" );
$Buffer->Write( "[" . localtime() . "] Service started\n" );

$Buffer->Write( "Log dir: $DB_DIR\n" );

@JobList = LaunchApps( \%List );
LogPids( @JobList );

while( SERVICE_STOPPED != ( $State = Win32::Daemon::State() ) )
{
    if( SERVICE_START_PENDING == $State )
    {
        # Initialization code
        Win32::Daemon::State( SERVICE_RUNNING );
		foreach my $Job ( @JobList )
		{
            $Buffer->Write( "\t$Job->{name}" );
			if( 1 == $Job->{process}->Wait( 0 ) )
            {
                # Process has terminated already.
                $Buffer->Write( " [terminated]" );
            }
            $Buffer->Write( "\n" );
		}
        $Buffer->Write( "[" . localtime() . "] Service initialized. Setting state to Running.\n\n" );
    }
	elsif( SERVICE_PAUSE_PENDING == $State )
	{
        $Buffer->Write( "[" . localtime() . "] Pausing...\n" );
		foreach my $Job ( @JobList )
		{
			if( 0 == $Job->{process}->Wait( 0 ) )
            {
                $Buffer->Write( "\t$Job->{name}\n" );
	    		$Job->{process}->Suspend();
            }
		}
		Win32::Daemon::State( SERVICE_PAUSED );
        $Buffer->Write( "\n" );
		next;
	}
	elsif( SERVICE_CONTINUE_PENDING == $State )
	{
        $Buffer->Write( "[" . localtime() . "] Resuming...\n" );
		foreach my $Job ( @JobList )
		{
            if( 0 == $Job->{process}->Wait( 0 ) )
            {
                $Buffer->Write( "\t$Job->{name}\n" );
			    $Job->{process}->Resume();
            }
		}
        Win32::Daemon::State( SERVICE_RUNNING );
        $Buffer->Write( "\n" );
		next;
	}
	elsif( SERVICE_STOP_PENDING == $State )
	{
        $Buffer->Write( "[" . localtime() . "] Stopping...\n" );
		foreach my $Job ( @JobList )
		{
            if( 0 == $Job->{process}->Wait( 0 ) )
            {
                $Buffer->Write( "\t$Job->{name}\n" );
			    $Job->{process}->Kill( 0 );
            }
		}
		Win32::Daemon::State( SERVICE_STOPPED );
        $Buffer->Write( "\n" );
		next;
	}

	sleep( $SLEEP_TIMEOUT );
}

print Win32::Daemon::StopService();
undef $Buffer;

sub LaunchApps
{
    my( $List ) = @_;
    my @Jobs;

    foreach ( keys( %$List ) )
    {
        my $App = $List->{$_};

        if( $App->{state} =~ /disabled/i )
        {
            print "\n$App->{name} is disabled. Skipping to next entry.\n";
        }
        else
        {
            print "\nLaunching: $App->{name}\n";
        
            push( @Jobs, $App ) if( Launch( $App ) );
        }
    }
    return( @Jobs );
}

sub LogPids
{
    my( @Jobs ) = @_;

    if( open( LOG, ">$DB_LOG" ) )
    {
	    map
	    {
		    print LOG "$_->{name}=$_->{pid}\n";
	    } @Jobs ;
	    close( FILE );
     }
}

sub Launch
{
    my( $App ) = @_;
	my $Process;
    my $iResult = 0;
    my( $Flags ) =  ($App->{flags}) | ($App->{priority});
    print "Flags=$Flags\n";

    print STDERR "\n  starting: $App->{program} $App->{params}";
    if( Win32::Process::Create(
                                $Process,
                                $App->{program},
                                "$App->{program} $App->{params}",
                                0 != $App->{inherit},
#                               $Flags,
                                16 | 32,
                                $App->{dir} ) )


    {
		$App->{process} = $Process;
		$App->{pid} = $Process->GetProcessID();
        print "  $App->{name} has been succesfully created.\n";
        $iResult = 1;
    }
    else
    {
        $iResult = 0;
        print "  Failed to launch: " . Win32::FormatMessage( Win32::GetLastError() ) . "\n";
    }
	return( $iResult );
}

sub ReadDB
{
	my( $FileName, $List ) = @_;
	my $Section = "";
    my $iCount = 0;

    if( open( FILE, "< $FileName" ) )
    {
        my( $Temp, $Process );
        
        foreach $Temp ( <FILE> )
    	{
    	    my( $Temp2, $Name, $Value );
    	    
    	    next if( $Temp =~ /^\s*?[;#]/ );
    	    if( ( $Temp2 ) = ( $Temp =~ /^\s*\[\s*(.*)\s*\]/ ) )
    	    {
    	        $iCount++;
    	        $Process = lc $Temp2;
    	        $List->{$Process}->{name}= $Temp2;
    	        next;
    	    }
	    
            ($Name, $Value ) = ($Temp =~ /\s*(.*?)\s*?=\s*(.*)/gi);
            $List->{$Process}->{lc $Name} = $Value if( $Name );
	    }

        close( FILE );
    }

    return( $iCount );
}

