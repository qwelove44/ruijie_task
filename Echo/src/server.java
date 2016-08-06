/**
 * 
 */

import java.net.*;
import java.io.*;
import java.util.concurrent.*;

class MyThread implements Runnable {
	Socket socket = null;
	public MyThread(Socket socket) {
		// TODO Auto-generated constructor stub
		this.socket = socket;
	}
	public MyThread() {
		this.socket = null;
	}
	private String process(String str) {
		String send_str = null;
		switch (ConfigThread.flag) {
		case 0:
			send_str = str;
			break;
		case 1:
			send_str = str.toUpperCase();
			break;
		case 2:
			send_str = str.toLowerCase();
			break;
		default:
			send_str = str;
			System.out.println("the flag param is wrong");
			break;
		}
		return send_str;
	}
	public void run() {
		try {
			PrintStream out = new PrintStream(this.socket.getOutputStream());
			BufferedReader buf =  new BufferedReader(new InputStreamReader(this.socket.getInputStream()));  
			while (true) {
				/*recv msg*/
				String recv_msg = buf.readLine();
				if (recv_msg == null || "".equals(recv_msg)) {
					System.out.println("client disconnect: ");
					break;
				}
				System.out.println("recv msg: " + recv_msg);
				
				/*send msg*/
				String send_msg = process(recv_msg);
				out.println(send_msg);
			}
			out.close();
			buf.close();
		} catch (Exception e) {
			// TODO: handle exception
			e.printStackTrace();
		} finally {
			try {
				this.socket.close();
			} catch (Exception e2) {
				// TODO: handle exception
				e2.printStackTrace();
			}
			
		}
	}
}

class ConfigThread implements Runnable {
	static int flag = 0;

	public void run() {
		try {
			BufferedReader input = new BufferedReader(new InputStreamReader(System.in)); 
			while (true) {
				System.out.println("SERVER> ");
				String configinform = input.readLine();
				if ("normal".equals(configinform)) {
					flag = 0;
				} else if ("upper".equals(configinform)) {
					flag = 1;
				} else if ("lower".equals(configinform)) {
					flag = 2;
				} else {
					System.out.println("mode param is wrong!");
				}
			}
		} catch (Exception e) {
			// TODO: handle exception
			e.printStackTrace();
		}
	}
}

public class server {
	
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		ServerSocket serversocket = null;
		Socket socket = null;
		int waittime = 500;
		
		ExecutorService executor = null;
		try {
			
			executor = Executors.newFixedThreadPool(echo_common.THREADMAX + 1);
			executor.execute(new ConfigThread());
			/*build connect*/
			serversocket = new ServerSocket(echo_common.PORT);
			while(true) {
				/*wait connect*/
				socket = serversocket.accept();
				System.out.println(socket.getInetAddress().getHostAddress());
				/*add tast*/
				executor.execute(new MyThread(socket)); 
			}

		} catch (Exception e) {
			// TODO: handle exception
			e.printStackTrace();
		} finally {
			try {
				
				socket.close();
				serversocket.close();
				executor.shutdown();
				executor.awaitTermination(waittime, TimeUnit.MILLISECONDS);
				
			} catch (Exception e2) {
				// TODO: handle exception
				e2.printStackTrace();
				
			}
		}
		
	}

}
