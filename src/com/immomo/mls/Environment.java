/**
  * Created by MomoLuaNative.
  * Copyright (c) 2019, Momo Group. All rights reserved.
  *
  * This source code is licensed under the MIT.
  * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
  */
package com.immomo.mls;

import org.luaj.vm2.Globals;

/**
 * Created by Xiong.Fangyu on 2019/3/25
 */
public class Environment {
    public static boolean DEBUG = false;

    public static final String LUA_ERROR = "[LUA_ERROR] ";

    public static UncatchExceptionListener uncatchExceptionListener = new UncatchExceptionListener() {
        @Override
        public boolean onUncatch(boolean fatal, Globals globals, Throwable e) {
            e.printStackTrace();
            return true;
        }
    };

    public static boolean callbackError(Throwable t, Globals g) {
        if (uncatchExceptionListener != null) {
            return uncatchExceptionListener.onUncatch(false, g, t);
        }
        return false;
    }

    public static interface UncatchExceptionListener {
        /**
         * called when some throwable is not caught in lua sdk
         *
         * @param e
         * @return true to handle uncatch throwable, false otherwise.
         */
        boolean onUncatch(boolean fatal, Globals globals, Throwable e);
    }
}