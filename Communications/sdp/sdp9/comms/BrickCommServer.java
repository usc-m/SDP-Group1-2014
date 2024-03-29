package sdp.sdp9.comms;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadFactory;

import sdp.comms.*;
import sdp.gui.SingletonRadio;


/*
 * 
 * For two-robot setup:
 * 		Rewrite the radio singleton so it is a "doubleton"
 * 		Rewrite BrickCommServer so it differentiates between the attacker and defender 
 */
public class BrickCommServer {
	Radio comm;
	private boolean connected;

	private ExecutorService executor;
	private List<StateChangeListener> stateChangeListeners;
	
	private final static ThreadFactory EXECUTOR_FACTORY = new ThreadFactory() {
		
		@Override
		public Thread newThread(Runnable runnable) {
			Thread t = new Thread(runnable, "BrickCommServer executor");
			t.setDaemon(true);
			return t;
		}
	};

	public BrickCommServer() {
		stateChangeListeners = new ArrayList<BrickCommServer.StateChangeListener>();
		connected = false;
		executor = Executors.newSingleThreadExecutor(EXECUTOR_FACTORY);
        //comm = new Radio("COM3");
		Radio.getPortNames();
        comm = new SingletonRadio("/dev/ttyACM0");
        
        //connect();
        
	}

	public void connect()  {
		System.out.print("Connecting");
		//comm.start();
		setConnected(true);
	}

	public boolean isConnected() {
		return connected;
	}
	
	private void setConnected(boolean connected) {
		if (this.connected != connected) {
			this.connected = connected;
			
			for (StateChangeListener listener : stateChangeListeners) {
				listener.stateChanged();
			}
		}
	}

	public void close() {
		if (comm != null)
			comm.stop();

		

		setConnected(false);
	}
	
	public void addStateChangeListener(StateChangeListener listener) {
		stateChangeListeners.add(listener);
	}
	
	public void removeStateChangeListener(StateChangeListener listener) {
		stateChangeListeners.remove(listener);
	}

	/**
	 * Executes a command asynchronously. Returns immediately and is safe to
	 * call from any thread.
	 */
	public void execute(final RobotCommand.Command command) {
		executor.execute(new Runnable() {

			@Override
			public void run() {
				BrickCommServer.this.executeSync(command);
			}
		});
	}

	/**
	 * Executes a command synchronously. Never call this method from GUI or
	 * frame grabber thread!
	 */
	public void executeSync(RobotCommand.Command command) {
		try {
			command.sendToBrick(comm);
		} catch (IOException e) {
			e.printStackTrace();
			close();
		}
	}

	// Legacy methods

	/*@Deprecated
	public void robotKick(int speed) throws IOException {
		brickOutput.writeInt(RobotOpcode.KICK);
		brickOutput.writeInt(speed);
		brickOutput.flush();
	}

	@Deprecated
	public void robotCatch() throws IOException {
		brickOutput.writeInt(RobotOpcode.CATCH);
		brickOutput.flush();
	}

	@Deprecated
	public void robotRotateBy(int angle, double speed) throws IOException {
		brickOutput.writeInt(RobotOpcode.ROTATE_BY);
		brickOutput.writeInt(angle);
		brickOutput.writeDouble(speed);
		brickOutput.flush();
	}

	@Deprecated
	public void robotArcForwards(double arcRadius, int distance, int speed)
			throws IOException {
		brickOutput.writeInt(RobotOpcode.ARC_FORWARDS);
		brickOutput.writeDouble(arcRadius);
		brickOutput.writeInt(distance);
		brickOutput.writeInt(speed);
		brickOutput.flush();
	}

	@Deprecated
	public void robotTravel(int distance, int travelSpeed) throws IOException {
		brickOutput.writeInt(RobotOpcode.TRAVEL);
		brickOutput.writeInt(distance);
		brickOutput.writeInt(travelSpeed);
		brickOutput.flush();

	}

	public boolean robotTest() throws IOException {
		brickOutput.writeInt(RobotOpcode.TEST);
		brickOutput.flush();
		boolean robotReceived = brickInput.readBoolean();
		return robotReceived;
	}

	public boolean robotTestINT(int param) throws IOException {
		brickOutput.writeInt(RobotOpcode.TESTINT);
		brickOutput.writeInt(param);
		brickOutput.flush();
		boolean robotReceived = brickInput.readBoolean();
		return robotReceived;
	}

	public boolean robotTestDOUBLE(double param) throws IOException {
		brickOutput.writeInt(RobotOpcode.TESTDOUBLE);
		brickOutput.writeDouble(param);
		brickOutput.flush();
		boolean robotReceived = brickInput.readBoolean();
		return robotReceived;
	}

	public boolean robotTestINTANDDOUBLE(int paramInt, double paramDouble)
			throws IOException {
		brickOutput.writeInt(RobotOpcode.TESTINTANDDOUBLE);
		brickOutput.writeDouble(paramDouble);
		brickOutput.writeInt(paramInt);
		brickOutput.flush();
		boolean robotReceived = brickInput.readBoolean();
		return robotReceived;
	}*/

	public interface StateChangeListener {
		void stateChanged();
	}
}
