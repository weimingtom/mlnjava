package com.example.mlnandroid;

import java.io.IOException;

public class MainActivity {
	public static void main(String[] args) {		
//		Globals.setAssetManagerForNative(this.getAssets());
		BridgeTest test = new BridgeTest();
		test.initPath();
		test.testMBit();
		test.onDestroy();
		try {
			System.in.read();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}
