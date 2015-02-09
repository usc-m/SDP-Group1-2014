package sdp.control;

import sdp.comms.Radio;

/**
 * Created by Matthew on 09/02/2015.
 */
public class HolonomicRobotController extends BaseRobotController {

    private int[] msPerCm;
    // Grey holonomic wheels are 58mm in diameter
    // http://www.microrobo.com/58mm-lego-compatible-omni-wheel.html
    private static final int WHEEL_DIAMETER = 58;
    private static final double WHEEL_CIRCUMFERENCE = Math.PI * WHEEL_DIAMETER;

    public HolonomicRobotController(Radio radio,
                                    boolean initialCatcher,
                                    int msPerCm_wheel1,
                                    int msPerCm_wheel2,
                                    int msPerCm_wheel3){
        super(radio, initialCatcher);

        msPerCm = new int[3];
        msPerCm[0] = msPerCm_wheel1;
        msPerCm[1] = msPerCm_wheel2;
        msPerCm[2] = msPerCm_wheel3;
    }

    @Override
    public void onMotionComplete() {

    }
}