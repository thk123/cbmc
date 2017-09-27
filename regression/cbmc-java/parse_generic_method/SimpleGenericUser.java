interface BasicInterface
{
  int getX();
}

class Foo implements BasicInterface
{
  public int x;

  public int getX() {
    return x;
  }
}

class Bar extends Foo
{}

class SimpleGeneric<T>
{
  public T t;

  public static <T> SimpleGeneric<T> makeGeneric(T value)
  {
    SimpleGeneric<T> newST = new SimpleGeneric<T>();
    newST.t = value;
    return newST;
  }
}

public class SimpleGenericUser
{

  public static <T> void processSimpleGeneric(SimpleGeneric<T> x) {
    assert(x.t==null);
  }

  // Test a wildcard generic bound by an interface
  public static <T extends BasicInterface> void processUpperBoundInterfaceGeneric(SimpleGeneric<T> x) {
    assert(x.t.getX() == 4);
  }

  // Test a wild card generic bound by a class
  public static <T extends Foo> void processUpperBoundClassGeneric(SimpleGeneric<T> x) {
    assert(x.t.getX() == 4);
  }

  public static <T extends Foo & BasicInterface> void processDoubleUpperBoundClassGeneric(SimpleGeneric<T> x) {
    assert(x.t.getX() == 4);
  }

  public static void test()
  {
    SimpleGeneric<Foo> myGenericValue = new SimpleGeneric<Foo>();
    myGenericValue.t = null;
    processSimpleGeneric(myGenericValue);

    myGenericValue.t = new Foo();
    myGenericValue.t.x = 4;
    processUpperBoundInterfaceGeneric(myGenericValue);

    SimpleGeneric<Bar> anotherGenericValue = new SimpleGeneric<Bar>();
    anotherGenericValue.t = new Bar();
    anotherGenericValue.t.x = 4;
    processUpperBoundClassGeneric(anotherGenericValue);

    SimpleGeneric<Bar> built = SimpleGeneric.<Bar>makeGeneric(new Bar());
  }
}