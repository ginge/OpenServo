system.print("Connecting to the servo controller:");
if(system.Connect()) {
	system.print("succeeded");
} else {
	system.print("failed");
}
