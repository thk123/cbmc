package com.diffblue.test.cbmc;

import com.thoughtworks.gauge.*;
import java.util.Iterator;
import java.util.regex.Pattern;
import org.json.simple.*;
import org.json.simple.parser.*;
import org.testng.asserts.*;
import static org.junit.Assert.assertEquals;
import com.diffblue.test.*;
import java.io.*;

public class TestCbmcRegression {

  private int timeout = 30;
  private String options = new String();
  private String file = new String();
  private String binary = new String();
  private String stdOut, stdErr;
  private int exitCode, signalCode;
  private SoftAssert result;

  @Step("Binary is <specLoc>")
  public void setBinaryLocation(String specLoc) {
    this.binary = System.getenv("CBMC_BASE") + "/" + specLoc;
  }

  @Step("Run with options <specParam> on file <file> in directory <dir>")
  public void setCbmcParameters(String specParam, String file, String dir) {
    this.options = specParam;
    this.file = file;
    ShellCmd runner = new ShellCmd();

    String cmd = binary + " " + " " + options + " " + file;
    runner.execShellCmd(cmd, System.getenv("CBMC_BASE") + '/' + dir);

    this.stdOut = runner.getStdOut();
    this.stdErr = runner.getStdErr();
    this.exitCode = runner.getExitCode();
    this.signalCode = runner.getSignalCode();
    Gauge.writeMessage("stdout follows");
    Gauge.writeMessage(this.stdOut);
    Gauge.writeMessage("stderr follows");
    Gauge.writeMessage(this.stdErr);
  }

  @ContinueOnFailure
  @Step("Exit code matches <val>")
  public void exitCode(String val) {
    SoftAssert results = new SoftAssert();
    Pattern p = Pattern.compile(val);
    results.assertTrue(p.matcher(Integer.toString(this.exitCode)).matches());
    results.assertAll();
  }

  @ContinueOnFailure
  @Step("Signal code matches <val>")
  public void signalCode(String val) {
    SoftAssert results = new SoftAssert();
    Pattern p = Pattern.compile(val);
    results.assertTrue(p.matcher(Integer.toString(this.signalCode)).matches());
    results.assertAll();
  }

  @ContinueOnFailure
  @Step("Match <regexp>")
  public void matchTest(String regexp) {
    SoftAssert results = new SoftAssert();
    Pattern p = Pattern.compile(regexp, Pattern.MULTILINE);
    results.assertTrue(p.matcher(this.stdErr).find());
    results.assertAll();
  }

  @ContinueOnFailure
  @Step("Do not match <regexp>")
  public void notMatchTest(String regexp) {
    SoftAssert results = new SoftAssert();
    Pattern p = Pattern.compile(regexp, Pattern.MULTILINE);
    results.assertTrue(!p.matcher(this.stdErr).find());
    results.assertAll();
  }

  @ContinueOnFailure
  @Step("Multiline match <regexp>")
  public void multilineMatchTest(String regexp) {
    SoftAssert results = new SoftAssert();
    Pattern p = Pattern.compile(regexp);
    results.assertTrue(p.matcher(this.stdErr).find());
    results.assertAll();
  }

  @ContinueOnFailure
  @Step("Multiline do not match <regexp>")
  public void multilineNotMatchTest(String regexp) {
    SoftAssert results = new SoftAssert();
    Pattern p = Pattern.compile(regexp);
    results.assertTrue(!p.matcher(this.stdErr).find());
    results.assertAll();
  }
}
