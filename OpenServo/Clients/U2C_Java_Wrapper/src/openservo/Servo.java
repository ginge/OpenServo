package openservo;

import i2c.*;
import java.awt.event.*;
import javax.swing.*;

/**
 * <p>Überschrift: U2C JNI-Wrapper</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright (c) 2006</p>
 * <p>Organisation: </p>
 * @author Stefan Engelke
 * @version 1.0
 */

public class Servo implements ActionListener {
  U2CDevice dev;
  U2CTransaction wCmd, rCmd;
  Timer timer;
  int cmdPos=0,pos=0;

  public Servo(U2CDevice dev,int i2cAdr) {
    this.dev=dev;
    wCmd=new U2CTransaction();
    wCmd.nSlaveDeviceAddress=i2cAdr;
    wCmd.nMemoryAddressLength=1;
    rCmd=new U2CTransaction();
    rCmd.nSlaveDeviceAddress=i2cAdr;
    rCmd.nMemoryAddressLength=1;
  }

  public void actionPerformed(ActionEvent event) {
    //pos=readPosition();
    ControlFrame.instance.posBar.setValue(pos);
    writePositionCmd(cmdPos);
  }

  public void start(int delay) {
    if (timer != null) timer.stop();
    timer = new Timer(delay, this);
    timer.start();
  }

  public void stop() {
    if (timer==null) return;
    timer.stop();
  }

  public void sendCommand(int cmdNr) {
    if (cmdNr>=0 && cmdNr<=2) {
      synchronized(wCmd) {
        wCmd.nMemoryAddress = 0x15;
        wCmd.nBufferLength = 3;
        wCmd.setUByte(0, cmdNr);
        wCmd.setUWord(1, 0);
        try {
          dev.write(wCmd);
        }
        catch (U2CSlaveException e) {
          e.printStackTrace();
          stop();
        }
      }
    }
  }

  public void setCmdPosition(int pos) { this.cmdPos=pos;
   // System.out.println(pos);
  }
  public int getPosition() { return pos; }

  private int readPosition() {
    rCmd.nMemoryAddress=0x08;
    rCmd.nBufferLength=2;
    try {
      dev.read(rCmd);
    } catch(U2CSlaveException e) {/*e.printStackTrace(); stop();*/ return 0; }
    return rCmd.getUWord(0);
  }

  private void writePositionCmd(int pos) {
    synchronized(wCmd) {
      wCmd.nMemoryAddress = 0x10;
      wCmd.nBufferLength = 2;
      wCmd.setUWord(0, pos);
      try {
        dev.write(wCmd);
      }
      catch (U2CSlaveException e) {
        e.printStackTrace();
        stop();
      }
    }
  }

}
