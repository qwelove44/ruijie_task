/**
 * 
 */

import java.net.*;
import java.io.*;


public class server {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		ServerSocket serversocket = null;
		Socket socket = null;
		OutputStream os = null;
		InputStream is = null;
		int port = 1234;
		
		try {
			/*build connect*/
			serversocket = new ServerSocket(port);
			
			socket = serversocket.accept();
			/*recv msg*/
			is = socket.getInputStream();
			byte[] recv_msg = new byte[1024];
			int recv_size = is.read(recv_msg);
			System.out.println("server recv msg: " + new String(recv_msg, 0, recv_size));
			/*send msg to client*/
			os = socket.getOutputStream();
			os.write(recv_msg, 0, recv_size);
			
		} catch (Exception e) {
			// TODO: handle exception
			e.printStackTrace();
		} finally {
			try {
				is.close();
				os.close();
				socket.close();
				serversocket.close();
			} catch (Exception e2) {
				// TODO: handle exception
				e2.printStackTrace();
			}
		}
		
	}

}
