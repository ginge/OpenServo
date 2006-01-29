package i2c;

/**
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


public class U2CException extends Exception {
  // Transaction result
  public final static int U2C_SUCCESS = 0;                            /*!< API Function was successfull */
  public final static int U2C_BAD_PARAMETER = 1;                      /*!< API Function got bad input parameter */
  public final static int U2C_HARDWARE_NOT_FOUND = 2;                 /*!< U2C Device not found */
  public final static int U2C_SLAVE_DEVICE_NOT_FOUND = 3;             /*!< I2C Slave device not found */
  public final static int U2C_TRANSACTION_FAILED = 4;                 /*!< I2C or SPI transaction failed */
  public final static int U2C_SLAVE_OPENNING_FOR_WRITE_FAILED = 5;    /*!< I2C Slave did not acknowledge write slave address */
  public final static int U2C_SLAVE_OPENNING_FOR_READ_FAILED = 6;     /*!< I2C Slave did not acknowledge read slave address */
  public final static int U2C_SENDING_MEMORY_ADDRESS_FAILED = 7;      /*!< I2C Slave did not acknowledge internal address output */
  public final static int U2C_SENDING_DATA_FAILED = 8;                /*!< I2C Slave did not acknowledge data output */
  public final static int U2C_NOT_IMPLEMENTED = 9;                    /*!< Operation is not implemented by U2C API */
  public final static int U2C_NO_ACK = 10;                            /*!< Got no ACK from Slave */
  public final static int U2C_DEVICE_BUSY = 11;                       /*!< U2C Device Busy */
  public final static int U2C_MEMORY_ERROR = 12;                      /*!< Memory operation (like malloc) failed */
  public final static int U2C_UNKNOWN_ERROR = 13;                     /*!< Undocumented error */
  public final static int U2C_I2C_CLOCK_SYNCH_TIMEOUT = 14;        /*!< I2C Slave did not release SCL line */


  public U2CException(String text) {
    super(text);
  }

  public static void checkDeviceStatus(int status) throws U2CDeviceException {
    switch(status) {
      case U2C_BAD_PARAMETER:
        throw new U2CDeviceException("API Function got bad input parameter");
      case U2C_HARDWARE_NOT_FOUND:
        throw new U2CDeviceException("U2C Device not found");
      case U2C_NOT_IMPLEMENTED:
        throw new U2CDeviceException("Operation is not implemented by U2C API");
      case U2C_DEVICE_BUSY:
        throw new U2CDeviceException("U2C Device Busy");
      case U2C_MEMORY_ERROR:
        throw new U2CDeviceException("Memory operation (like malloc) failed");
      case U2C_UNKNOWN_ERROR:
        throw new U2CDeviceException("Undocumented error");
    }
    return;
  }

  public static void checkSlaveStatus(int status) throws U2CSlaveException {
    switch(status) {
      case U2C_SLAVE_DEVICE_NOT_FOUND:
        throw new U2CSlaveException("I2C Slave device not found");
      case U2C_TRANSACTION_FAILED:
        throw new U2CSlaveException("I2C or SPI transaction failed");
      case U2C_SLAVE_OPENNING_FOR_WRITE_FAILED:
        throw new U2CSlaveException("I2C Slave did not acknowledge write slave address");
      case U2C_SLAVE_OPENNING_FOR_READ_FAILED:
        throw new U2CSlaveException("I2C Slave did not acknowledge read slave address");
      case U2C_SENDING_MEMORY_ADDRESS_FAILED:
        throw new U2CSlaveException("I2C Slave did not acknowledge internal address output");
      case U2C_SENDING_DATA_FAILED:
        throw new U2CSlaveException("I2C Slave did not acknowledge data output");
      case U2C_NO_ACK:
        throw new U2CSlaveException("Got no ACK from Slave");
      case U2C_I2C_CLOCK_SYNCH_TIMEOUT:
        throw new U2CSlaveException("I2C Slave did not release SCL line");
    }
    return;
  }

}
