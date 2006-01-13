// if we are not connected to the servo controller do so now
if(!system.Connect()) {
	system.trace("connection to the servo controller failed.");
}

// we must have more than one servo for this script to work!
controller.scan();
if(controller.count>1) {
	var servo1 = controller.item(0);
	var servo2 = controller.item(1);
	system.print("This script sets one servo to follow the other!");
//	system.print("Servo "+servo2.typeName+":"+servo2.address+" will follow servo "+servo1.address);
	
	servo1.enabled = false;
	servo2.enabled = true;

	var last_position;		// save last position, no need to update if already at target position
	if(servo1 && servo2) {
		do {
			var position = servo1.position;
			if( last_position != position) {	// has position changed?
				servo2.position = last_position = position;
				system.trace("position = "+position);
			}
			system.sleep(100);	// wait 100 milliseconds
		} while(1);	// loop forever
	}
} else
	system.print("There must be more than two servos to run this script.");