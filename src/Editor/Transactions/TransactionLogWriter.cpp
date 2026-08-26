#include "TransactionLogWriter.h"
#include "Logger.h"

namespace
{
const char* opKindToString(eTransactionOpKind kind)
{
	switch (kind)
	{
	case eTransactionOpKind::setProperty:
		return "set";
	case eTransactionOpKind::createObject:
		return "create";
	case eTransactionOpKind::destroyObject:
		return "destroy";
	default:
		return "unknown";
	}
}
} // namespace

bool TransactionLogWriter::open(const std::filesystem::path& projectFilePath)
{
	const std::filesystem::path logFolder= projectFilePath.parent_path() / "logs";
	if (!openLogFile(logFolder, "transactions"))
	{
		return false;
	}

	configuru::Config sessionLine= configuru::Config::object();
	sessionLine["event"]= "session";
	sessionLine["project"]= projectFilePath.string();
	sessionLine["started"]= makeLocalTimestampString("%Y-%m-%dT%H:%M:%S");
	writeLine(sessionLine);

	MIKAN_LOG_INFO("TransactionLogWriter") << "Transaction log: " << getLogFilePath().string();
	return true;
}

void TransactionLogWriter::writeTransaction(const Transaction& transaction)
{
	if (!isOpen())
		return;

	configuru::Config opsArray= configuru::Config::array();
	for (const TransactionOp& op : transaction.ops)
	{
		configuru::Config opConfig= configuru::Config::object();
		opConfig["op"]= opKindToString(op.kind);
		opConfig["system"]= op.systemName;
		if (!op.componentClassName.empty())
			opConfig["class"]= op.componentClassName;
		opConfig["id"]= op.componentId;

		if (op.kind == eTransactionOpKind::setProperty)
		{
			opConfig["prop"]= op.propertyName;
			opConfig["type"]= mikanVariantTypeToString(op.valueType);
			opConfig["old"]= op.oldValueText;
			opConfig["new"]= op.newValueText;
		}
		else
		{
			opConfig["def"]= op.definitionConfig;
		}

		opsArray.push_back(opConfig);
	}

	configuru::Config txnLine= configuru::Config::object();
	txnLine["event"]= "txn";
	txnLine["seq"]= transaction.sequenceNumber;
	txnLine["t"]= transaction.timestampMs;
	txnLine["desc"]= transaction.description;
	if (!transaction.gestureId.empty())
		txnLine["gesture"]= transaction.gestureId;
	txnLine["ops"]= opsArray;
	writeLine(txnLine);
}
