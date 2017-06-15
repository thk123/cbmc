package com.diffblue.test;

import java.io.*;
import com.thoughtworks.gauge.*;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

public class ShellCmd {
  private String stdOut = "";
  private String stdErr = "";

  //An invalid signal value
  private final int INVALIDSIGNALCODE = 999;
  //An invalid exit value
  private final int INVALIDEXITCODE = 999;
  //This is maximum exit code possible
  private final int MAXEXITCODE = 255;
  //This is the maximum self termination value for an application
  private final int MAXAPPEXITCODE = 127;
  //This is the exit value for an application that successfully terminates
  private final int ERRORFREEEXITCODE = 0;

  private Integer exitCode = INVALIDEXITCODE;
  private Integer signalCode = INVALIDSIGNALCODE;

  private void setStdOut(String newStdOut) {
    stdOut = newStdOut;
  }

  private void setStdErr(String newStdErr) {
    stdErr = newStdErr;
  }

  private void setExitCode(Integer newExitCode) {
    if (newExitCode > MAXEXITCODE) {
      // This is an invalid exit code
      exitCode = INVALIDEXITCODE;
      signalCode = INVALIDSIGNALCODE;
    } else {
      exitCode = newExitCode;
      if (exitCode > MAXAPPEXITCODE ) {
        signalCode = exitCode;
      } else {
        signalCode = ERRORFREEEXITCODE;
      }
    }
  }

  public String getStdOut() {
    return stdOut;
  }

  public String getStdErr() {
    return stdErr;
  }

  public int getExitCode() {
    return (int) exitCode;
  }

  public int getSignalCode() {
    return (int) signalCode;
  }

  public void execShellCmd(String cmd) {
    execShellCmd(cmd, "");
  }

  public void execShellCmd(String cmd, String dir) {
    execShellCmd(cmd, dir, true);
  }

  public void execShellCmd(String cmd, String dir, Boolean waitFor) {
    // Run the command and return the exit status.
    Process runCmd;
    String stdOut = new String();
    String stdErr = new String();
    String line = new String();

    Gauge.writeMessage("Command is: " + cmd);
    Gauge.writeMessage("Working dir is: " + dir);
    Gauge.writeMessage("WaitFor is: " + waitFor);

    try {
      if (dir.equals("")) {
        runCmd = Runtime.getRuntime().exec(cmd);
      } else {
        File dirFile = new File(dir);
        runCmd = Runtime.getRuntime().exec(cmd,null,dirFile);
      }

      // Default is to wait for the process to finish. If not waiting for
      // process to finish no point collecting the output
      if (waitFor) {
        stdOut = readStream(runCmd.getInputStream());
        stdErr = readStream(runCmd.getErrorStream());

        runCmd.waitFor();

        setStdOut(stdOut);
        setStdErr(stdErr);
        setExitCode(runCmd.exitValue());
      }
    } catch (IOException e) {
      System.out.println(e.toString());
      Gauge.writeMessage("**** IOException in shellCmd ****");
      Gauge.writeMessage("Standard output\n" + getStdOut());
      Gauge.writeMessage("Standard Error\n" + getStdErr());
      Gauge.writeMessage("Exit Code: " + getExitCode());
      Gauge.writeMessage("Stack Trace\n" + e.getMessage());
      // If we get an exception make sure we record this as a failure
      fail();
    } catch(InterruptedException e) {
      // If we get an exception make sure we record this as a failure
      Gauge.writeMessage("**** InterruptedException in shellCmd ****");
      Gauge.writeMessage("Standard output\n" + getStdOut());
      Gauge.writeMessage("Standard Error\n" + getStdErr());
      Gauge.writeMessage("Exit Code: " + getExitCode());
      Gauge.writeMessage("Stack Trace\n" + e.getMessage());
      // If we get an exception make sure we record this as a failure
      fail();
    }
  }

  private String readStream(InputStream stream) {
    String shellData = "";
    String line = "";

    try {
      BufferedReader std = new BufferedReader(new InputStreamReader(stream));
      while ((line = std.readLine()) != null) {
        shellData += line + "\n";
      }
    } catch (IOException e) {
      Gauge.writeMessage(
          "**** Exception in streamReader function for reading standard output or error ****");
      assertEquals(e.getStackTrace(),"");
      return shellData;
    }
    return shellData;
  }
}