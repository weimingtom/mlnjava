package com.immomo.android.util;

public class Log {
	public static void e(String tag, String content) {
		System.out.println("[error][" + tag + "] " + content);
	}
	
	public static void d(String tag, String content) {
		System.err.println("[debug][" + tag + "] " + content);
	}	

	public static void i(String tag, String content) {
		System.err.println("[info][" + tag + "] " + content);
	}	
	
	
	public static void e(String tag, String content, Throwable e) {
		System.err.println("[error][" + tag + "] " + content);
		e.printStackTrace();
	}
	
	public static void w(String tag, String content, Throwable e) {
		System.err.println("[warning][" + tag + "] " + content);
		e.printStackTrace();
	}
}
