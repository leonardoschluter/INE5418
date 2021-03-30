import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject; 
import java.util.HashMap;

public class Calculator extends UnicastRemoteObject implements CalculatorInterface{
    int count = 0 ;
	private Object mutex = new Object();
	public Calculator() throws RemoteException {
    this.count = 0 ;
	this.mutex = new Object();
	}
	
	@Override
	public Integer getCount() throws RemoteException {
        synchronized (this.mutex) {
            this.count = this.count + 1;
		    return this.count;
        }
	}
}
