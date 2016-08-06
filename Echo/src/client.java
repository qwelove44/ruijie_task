/*
 * Copyright(C) 2016 Ruijie Network. All rights reserved.
 */
/*
 * client.c
 * Original Author:  liyonghua@ruijie.com.cn, 2016-08-4
 *
 * echo client
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
		Socket client_socket  = null;
		
		try {
			client_socket = new Socket(echo_common.SERVER_ADDR, echo_common.PORT);
			BufferedReader input = new BufferedReader(new InputStreamReader(System.in)); 
			PrintStream out = new PrintStream(client_socket.getOutputStream());
			BufferedReader buf =  new BufferedReader(new InputStreamReader(client_socket.getInputStream()));  
			/*new build connect*/
			
			while (true) {
				/*send msg*/
				System.out.println("CLIENT> ");
				String send_msg = input.readLine();
				if ("quit".equals(send_msg)) {
					System.out.println("client break connect");
					break;
				}
				out.println(send_msg);
				
				/*recv msg*/
				String recv_msg = buf.readLine();
				if (recv_msg == null || "".equals(recv_msg)) {
					System.out.println("server disconnect!");
					break;
				}
				System.out.println("echo msg: " + recv_msg);
				
			}
		} catch (Exception e) {
			// TODO: handle exception
			e.printStackTrace();
		} finally {
			try {
				client_socket.close();
			} catch (Exception e2) {
				// TODO: handle exception
				e2.printStackTrace();
			}
		}
	}

}
