import i2c.*;
import java.net.*;

/**
 * <P>U2C-Wrapper Test-Application</p>
 *
 * <hr>
 * <p>Copyright (c) 2006, Stefan Engelke (contact@stefanengelke.de)<br>
 * All rights reserved.</p>
 *
 * <p>Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:</p>
 *
 * <p>* Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.</p>
 *
 * <p>* Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.</p>
 *
 * <p>* Neither the name of the copyright holders nor the names of
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.</p>
 *
 *  <p>THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.</p>
 *
 * @author Stefan Engelke
 * @version 1.0
 */



public class Test {
  public Test() {
    // Reading dll-version
    U2CVersionInfo ver=U2CWrapper.getDllVersion();
    System.out.println("I2CBrdg.dll version: "+ver.majorVersion+"."+ver.minorVersion);

    // Counting U2C-devices
    int deviceCount=U2CWrapper.getDeviceCount();
    System.out.println(deviceCount+" device(s) detected on current computer");

    // If there is no device detected then exit
    if (deviceCount==0) return;

    // Opening the first detected device
    U2CDevice dev;
    System.out.println("opening first device..");
    try {
      dev=U2CWrapper.openDevice(0);
    } catch(U2CDeviceException e) {e.printStackTrace(); return;}

    // Reading driver-version
    ver=dev.getDriverVersion();
    System.out.println("driver version: "+ver.majorVersion+"."+ver.minorVersion);

    // Reading firmware-version
    ver=dev.getFirmwareVersion();
    System.out.println("firmware version: "+ver.majorVersion+"."+ver.minorVersion);

    // Scanning for I2C-slaves
    System.out.println("scanning for I2C slaves..");
    U2CSlaveAdrList slaves = dev.scanForSlaves();
    System.out.println(slaves.nDeviceNumber+" I2C Slaves found:");
    for (int i=0;i<slaves.nDeviceNumber;i++) System.out.println("  # "+slaves.list[i]);

    // If there is no slave found, exit
    if (slaves.nDeviceNumber==0) return;

    // Sending test command to first detected slave
    System.out.println("sending test command..  (OpenServo cmd-position)");
    U2CTransaction cmd=new U2CTransaction();
    cmd.nSlaveDeviceAddress=slaves.list[0];
    cmd.nMemoryAddressLength=1;
    cmd.nMemoryAddress=0x10;
    cmd.nBufferLength=2;
    cmd.setUWord(0,100);
    try {
      dev.write(cmd);
    } catch(U2CSlaveException e) { e.printStackTrace(); }

    // Reading register
    System.out.println("reading test registers..  (OpenServo current-position)");
    cmd.nMemoryAddress=0x08;
    cmd.nBufferLength=2;
    try {
      dev.read(cmd);
    } catch(U2CSlaveException e) { e.printStackTrace(); }
    System.out.println("  REG_POSITION_HI = "+cmd.getHex(0));
    System.out.println("  REG_POSITION_LO = "+cmd.getHex(1));
    System.out.println("  POSTION         = "+cmd.getUWord(0));

    // Closing device
    dev.close();
  }
  public static void main(String[] args) {
    Test test1 = new Test();
  }

}
