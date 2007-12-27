package openservo;

import javax.swing.*;
import java.awt.*;
import com.borland.jbcl.layout.*;
import javax.swing.border.*;
import java.awt.event.*;
import i2c.*;
import javax.swing.event.*;

/**
 * <p>Überschrift: U2C JNI-Wrapper</p>
 * <p>Beschreibung: </p>
 * <p>Copyright: Copyright (c) 2006</p>
 * <p>Organisation: </p>
 * @author Stefan Engelke
 * @version 1.0
 */

public class ControlFrame extends JFrame {
  JPanel panelControls = new JPanel();
  JSlider posSlider = new JSlider();
  JProgressBar posBar = new JProgressBar();
  GridLayout gridLayout1 = new GridLayout();
  JLabel posLabel = new JLabel();
  TitledBorder titledBorder1;
  TitledBorder titledBorder2;
  Servo servo;
  U2CDevice u2c;
  static ControlFrame instance;
  JPanel panelStateControl = new JPanel();
  JLabel stateLabel = new JLabel();
  JLabel labelCmdNr = new JLabel();
  JTextField tfCmdNr = new JTextField();
  JButton cbSend = new JButton();
  public ControlFrame() {
    instance=this;
    if (U2CWrapper.getDeviceCount()==0) {
      System.out.println("no U2C-device found..");
      System.exit(0);
    }
    try {
      u2c = U2CWrapper.openDevice(0);
    } catch(U2CDeviceException e) { e.printStackTrace(); System.exit(0); }
    u2c.setI2cFreq(U2CWrapper.U2C_I2C_FREQ_STD);
    servo=new Servo(u2c,16);
    servo.start(2);
    try {
      jbInit();
    }
    catch(Exception e) {
      e.printStackTrace();
    }

  }
  private void jbInit() throws Exception {
    titledBorder1 = new TitledBorder("");
    titledBorder2 = new TitledBorder("");
    this.getContentPane().setLayout(null);
    panelControls.setBorder(BorderFactory.createRaisedBevelBorder());
    panelControls.setBounds(new Rectangle(3, 23, 388, 84));
    panelControls.setLayout(gridLayout1);
    gridLayout1.setColumns(1);
    gridLayout1.setRows(3);
    posLabel.setBorder(BorderFactory.createEtchedBorder());
    posLabel.setDoubleBuffered(false);
    posLabel.setHorizontalAlignment(SwingConstants.CENTER);
    posLabel.setText("Servo Position");
    this.setDefaultCloseOperation(EXIT_ON_CLOSE);
    this.setResizable(false);
    this.setTitle("OpenServo-Control");
    posSlider.setMaximum(1100);
    posSlider.setPaintLabels(false);
    posSlider.setPaintTicks(false);
    posSlider.setPaintTrack(true);
    posSlider.addChangeListener(new ControlFrame_posSlider_changeAdapter(this));
    posBar.setMaximum(1100);
    panelStateControl.setLayout(null);
    panelStateControl.setBounds(new Rectangle(3, 110, 388, 84));
    panelStateControl.setBorder(BorderFactory.createRaisedBevelBorder());
    stateLabel.setText("State Command");
    stateLabel.setBounds(new Rectangle(2, 2, 384, 26));
    stateLabel.setHorizontalAlignment(SwingConstants.CENTER);
    stateLabel.setDoubleBuffered(false);
    stateLabel.setBorder(BorderFactory.createEtchedBorder());
    labelCmdNr.setText("Cmd Nr.");
    labelCmdNr.setBounds(new Rectangle(81, 44, 57, 21));
    tfCmdNr.setText("1");
    tfCmdNr.setBounds(new Rectangle(142, 42, 63, 23));
    cbSend.setBounds(new Rectangle(213, 41, 92, 24));
    cbSend.setText("send");
    cbSend.addActionListener(new ControlFrame_cbSend_actionAdapter(this));
    panelControls.add(posLabel, null);
    this.getContentPane().add(panelControls, null);
    panelControls.add(posSlider, null);
    panelControls.add(posBar, null);
    this.getContentPane().add(panelStateControl, null);
    panelStateControl.add(stateLabel, null);
    panelStateControl.add(cbSend, null);
    panelStateControl.add(tfCmdNr, null);
    panelStateControl.add(labelCmdNr, null);
  }

  public static void main(String[] args) {
    ControlFrame frame=new ControlFrame();
    frame.setBounds(200,200,400,300);
    frame.show();
  }

  void posSlider_stateChanged(ChangeEvent e) {
    servo.setCmdPosition(posSlider.getValue());
  }

  void cbSend_actionPerformed(ActionEvent e) {
    servo.sendCommand(Integer.parseInt(tfCmdNr.getText()));
  }



}

class ControlFrame_posSlider_changeAdapter implements javax.swing.event.ChangeListener {
  ControlFrame adaptee;

  ControlFrame_posSlider_changeAdapter(ControlFrame adaptee) {
    this.adaptee = adaptee;
  }
  public void stateChanged(ChangeEvent e) {
    adaptee.posSlider_stateChanged(e);
  }
}

class ControlFrame_cbSend_actionAdapter implements java.awt.event.ActionListener {
  ControlFrame adaptee;

  ControlFrame_cbSend_actionAdapter(ControlFrame adaptee) {
    this.adaptee = adaptee;
  }
  public void actionPerformed(ActionEvent e) {
    adaptee.cbSend_actionPerformed(e);
  }
}
