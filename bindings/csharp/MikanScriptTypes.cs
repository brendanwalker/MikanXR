namespace MikanXR
{
	public class MikanScriptMessageInfo : MikanEvent
	{
		public string content { get; set; }

		public MikanScriptMessageInfo() : base(typeof(MikanScriptMessageInfo).Name) {}
	};
}