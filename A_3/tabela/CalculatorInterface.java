import java.rmi.Remote;
import java.rmi.RemoteException;

public interface CalculatorInterface extends Remote{

	public Integer getCount() throws RemoteException;
}
