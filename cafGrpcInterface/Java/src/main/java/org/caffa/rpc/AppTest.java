//##################################################################################################
//
//   Caffa
//   Copyright (C) 2021- 3D-Radar
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
package org.caffa.rpc;

import org.caffa.rpc.AppInfo;
import org.caffa.rpc.AppGrpc;

import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;
import io.grpc.Status;
import io.grpc.StatusRuntimeException;
import com.google.protobuf.Empty;

import java.util.logging.Logger;

public class AppTest {
    private static final Logger logger = Logger.getLogger(AppTest.class.getName());
    private final AppGrpc.AppBlockingStub appStub;
    private final ManagedChannel channel;

    public AppTest(String host, int port) {
        this.channel = ManagedChannelBuilder.forAddress(host, port).usePlaintext().build();
        this.appStub = AppGrpc.newBlockingStub(channel);
    }

    public String appName()
    {
        Empty message = Empty.getDefaultInstance();
        AppInfoReply appInfo = this.appStub.getAppInfo(message);
        StringBuilder sb = new StringBuilder();
        sb.append(appInfo.getName());
        sb.append(" version ");
        sb.append(appInfo.getMajorVersion());
        sb.append(".");
        sb.append(appInfo.getMinorVersion());
        return sb.toString();
    }

    public static void main(String[] args) throws InterruptedException {
        AppTest test = new AppTest("localhost", 55555);
        String appName = test.appName();
        logger.info("Application Name and Version: " + appName);
    }
}
