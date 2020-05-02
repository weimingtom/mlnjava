package com.example.mlnandroid;

import org.luaj.vm2.Globals;
import org.luaj.vm2.jse.JSERegister;
import org.luaj.vm2.utils.IGlobalsUserdata;
import org.luaj.vm2.utils.PathResourceFinder;

import com.immomo.android.util.Log;

import com.immomo.mls.wrapper.Register;

public class BridgeTest {
	protected Globals globals;
	protected Register register;	
	
//	@Before
	public void initPath() {
//		Utils.loadLibrary();
		assertTrue(Globals.isInit());
		register = new Register();
		Log.i("BaseLuaTest", "Lua vm is " + (Globals.isIs32bit() ? "32" : "64") + " bits");
	}	
	
	protected void initGlobals(boolean withLooper) {
		globals = Globals.createLState(true);
		globals.setBasePath(Utils.getAssetsPath(), false);
		globals.setResourceFinder(new PathResourceFinder(Utils.getAssetsPath()));
		globals.setJavaUserdata(new IGlobalsUserdata() {
			@Override
			public void onGlobalsDestroy(Globals g) {

			}

			@Override
			public void l(long L, String tag, String log) {
				Log.i(tag, log);
			}

			@Override
			public void e(long L, String tag, String log) {
				Log.e(tag, log);
			}
		});
		Log.i("BridgeTest", "---------------on Start---------------");
		registerBridge();
		register.install(globals);
	}	
	
	protected void registerBridge() {
		JSERegister.registerLuaJava(globals);
	}	
	
	protected void checkStackSize(int size) {
		assertEquals(size, globals.dump().length);
	}	
	
	void assertEquals(int x, int y) {
		if (x != y) {
			Log.e("BridgeTest", globals.getErrorMsg());
			throw new RuntimeException("assertEquals");
		}
	}
	
	void assertTrue(boolean x) {
		if (!x) {
			Log.e("BridgeTest", globals.getErrorMsg());
			throw new RuntimeException("assertTrue");
		}
	}
	
    //private static final String MBitPath = "test_bit.lua";	
    private static final String MBitPath = "test_luajava.lua";	
    public void testMBit() {
        initGlobals(false);
        String path = /*Utils.getAssetsPath() + File.separator + */MBitPath;

//        assertTrue(globals.loadAssetsFile(path, MBitPath));
        assertTrue(globals.loadFile(path, MBitPath));
        assertTrue(globals.callLoadedData());

        checkStackSize(1);
    }
    
//    @After
    public void onDestroy() {
        Log.i("BreidgeTest", "---------------onDestroy---------------");
        if (globals != null)
        	globals.destroy();
        Utils.onGlobalsDestroy();
    }    
}
