package i2c;

/**
 * <P>This class handles all U2C-Device functions.</P>
 * <p>To get a U2CDevice-Object use the U2CWrapper class:<br>
 * {@link i2c.U2CWrapper#openDevice(int nDevice)}<br>
 * {@link i2c.U2CWrapper#openDeviceBySerialNum(long nSerialNum)}</p>
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

public class U2CDevice {
  private long handler=-1;
  private long serial=-1;

  protected U2CDevice() {  }

  protected long getHandler() { return handler; }
  protected void setHandler(long h) { handler=h; }
  protected void setSerial(long serial) { this.serial=serial; }

  /**
   * Returns the device serial number.
   * @return long, serial number
   */
  public long getSerial() {
    if (serial==-1) {
      try { serial = U2CWrapper.getSerialNum(this); } catch(Exception e) {e.printStackTrace();}
    }
    return serial;
  }

  /**
   * Returns the driver version.
   * @return U2CVersionInfo, driver version
   */
  public U2CVersionInfo getDriverVersion() {
    U2CVersionInfo version=null;
    try {
      version=U2CWrapper.getDriverVersion(this);
    } catch(U2CDeviceException e) { e.printStackTrace(); }
    return version;
  }

  /**
   * Returns the firmware version
   * @return U2CVersionInfo, firmware version
   */
  public U2CVersionInfo getFirmwareVersion() {
    U2CVersionInfo version=null;
    try {
      version=U2CWrapper.getFirmwareVersion(this);
    } catch(U2CDeviceException e) { e.printStackTrace(); }
    return version;
  }

  /**
   * Checks if I2C-clock-synchronisation is enabled
   * @return boolean, enabled
   */
  public boolean isClockSynchEnabled() {
    boolean enabled=false;
    try {
      enabled=U2CWrapper.getClockSynch(this);
    } catch(U2CDeviceException e) { e.printStackTrace(); }
    return enabled;
  }

  /**
   * Enable or disable the I2C-clock-synchronisation
   * @param enable boolean
   */
  public void setClockSynchEnabled(boolean enable) {
    try {
      U2CWrapper.setClockSynch(this,enable);
    } catch(U2CDeviceException e) { e.printStackTrace(); }
  }

  /**
   * Obtains I2C bus frequency.
   * @return I2C bus frequency. See {@link i2c.U2CDevice#setI2cFreq(int frequency)}
   */
  public int getI2cFreq() {
    int freq=0;
    try {
      freq=U2CWrapper.getI2cFreq(this);
    } catch(U2CDeviceException e) { e.printStackTrace(); }
    return freq;
  }

  /**
   * Configures I2C bus frequency.
   * @param freq The frequency of I2C bus, where:<br>
   * 0 corresponds to I2C bus fast mode (400 kHz).<br>
   * 1 corresponds to I2C bus standard mode (100 kHz).<br>
   * 1+n corresponds to clock period of I2C bus equal to 10 + 2*n uS.<br>
   * For convenience following constants were introduced:<br>
   * U2CWrapper.U2C_I2C_FREQ_FAST    I2C bus fast mode (400 kHz)<br>
   * U2CWrapper.U2C_I2C_FREQ_STD     I2C bus standard mode (100 kHz)<br>
   * U2CWrapper.U2C_I2C_FREQ_83KHZ   83 kHz<br>
   * U2CWrapper.U2C_I2C_FREQ_71KHZ   71 kHz<br>
   * U2CWrapper.U2C_I2C_FREQ_62KHZ   62 kHz<br>
   * U2CWrapper.U2C_I2C_FREQ_50KHZ   50 kHz<br>
   * U2CWrapper.U2C_I2C_FREQ_25KHZ   25 kHz<br>
   * U2CWrapper.U2C_I2C_FREQ_10KHZ   10 kHz<br>
   * U2CWrapper.U2C_I2C_FREQ_5KHZ     5 kHz<br>
   * U2CWrapper.U2C_I2C_FREQ_2KHZ     2 kHz<br>
   */
  public void setI2cFreq(int freq) {
    try {
      U2CWrapper.setI2cFreq(this,freq);
    } catch(U2CDeviceException e) { e.printStackTrace(); }
  }

  /**
   * Reads up to 256 bytes from the I2C slave device.
   * @param cmd U2CTransaction to be used during the I2C read transaction.
   * @throws U2CSlaveException
   */
  public void read(U2CTransaction cmd) throws U2CSlaveException {
    try {
      U2CWrapper.read(this,cmd);
    } catch(U2CDeviceException e) { e.printStackTrace(); }
  }

  /**
   * Writes up to 256 bytes to a I2C slave device.
   * @param cmd U2CTransaction to be used during the I2C write transaction
   * @throws U2CSlaveException
   */
  public void write(U2CTransaction cmd) throws U2CSlaveException {
    try {
      U2CWrapper.write(this,cmd);
    } catch(U2CDeviceException e) { e.printStackTrace(); }
  }

  /**
   * Scans slave device addresses currently occupied by I2C slave devices
   * connected to the I2C bus.
   * @return List of slave devices.
   */
  public U2CSlaveAdrList scanForSlaves() {
    U2CSlaveAdrList slaves=null;
    try {
      slaves=U2CWrapper.scanForSlaves(this);
    } catch(U2CDeviceException e) { e.printStackTrace(); }
    return slaves;
  }

  /**
   * Closes this U2C-device.
   */
  public void close() {
    try {
      U2CWrapper.closeDevice(this);
    } catch(U2CDeviceException e) { e.printStackTrace(); }
  }

}
