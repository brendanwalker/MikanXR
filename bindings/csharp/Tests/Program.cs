using System;

namespace MikanXR
{
	class Program
	{
		static int Main(string[] args)
		{
			bool success = true;
			Console.WriteLine("Running Unit Tests.");

			SerializationUnitTests serializationTests = new SerializationUnitTests();
			success &= serializationTests.RunAllTests();

			if (success)
			{
				Console.WriteLine("All Unit Tests Passed.");
			}
			else
			{
				Console.WriteLine("Some Unit Tests Failed!");
			}

			return success ? 0 : 1;
		}
	}
}
