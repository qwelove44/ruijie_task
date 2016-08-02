/**
 * 
 */

import java.net.*;
import java.io.*;

public class client {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		Socket socket  = null;
		InputStream is = null;
		OutputStream os = null;
		String server_ip = "127.0.0.1";
		int port = 1234;
		String msg = "hehe";
		try {
			/*new build connect*/
			socket = new Socket(server_ip, port);
			
			/*send msg*/
			os = socket.getOutputStream();
			os.write(msg.getBytes());
			
			/*recv msg*/
			is = socket.getInputStream();
			byte[] recv_msg = new byte[1024];
			int recv_size = is.read(recv_msg);
			System.out.println("recv msg: " + new String(recv_msg, 0, recv_size));
			
		} catch (Exception e) {
			// TODO: handle exception
			e.printStackTrace();
		} finally {
			try {
				is.close();
				os.close();
				socket.close();
			} catch (Exception e2) {
				// TODO: handle exception
				e2.printStackTrace();
			}
		}
	}

}
