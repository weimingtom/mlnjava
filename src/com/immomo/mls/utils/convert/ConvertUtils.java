/**
  * Created by MomoLuaNative.
  * Copyright (c) 2019, Momo Group. All rights reserved.
  *
  * This source code is licensed under the MIT.
  * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
  */
package com.immomo.mls.utils.convert;

import org.luaj.vm2.Globals;
import org.luaj.vm2.LuaTable;
import org.luaj.vm2.LuaValue;

import com.immomo.mls.wrapper.Translator;

/**
 * Created by XiongFangyu on 2018/7/26.
 * <p>
 * 数据类型转换工具
 *
 * @see #toNativeValue(LuaValue)
 * @see #toLuaValue(Globals, Object)
 * @see #toMap(LuaTable)
 */
public class ConvertUtils {
    public static LuaValue toLuaValue(Globals globals, Object value) {
        LuaValue ret = Translator.isPrimitiveLuaData(value) ? Translator.translatePrimitiveToLua(value) : null;
        if (ret == null)
            ret = Translator.translateJavaToLua(globals, value);
        return ret;
    }
}