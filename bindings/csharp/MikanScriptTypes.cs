namespace MikanXR
{
	struct MikanScriptMessageInfo : public MikanEvent
	{
		std::string content { get; set; }
	};
}