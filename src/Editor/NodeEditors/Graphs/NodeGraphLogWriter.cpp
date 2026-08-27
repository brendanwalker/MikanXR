#include "NodeGraphLogWriter.h"
#include "Logger.h"

namespace
{
struct SnapshotCollection
{
	const char* jsonKey;
	const char* label;
};

// Indices match NodeGraphLogWriter::m_lastCounts
const SnapshotCollection k_snapshotCollections[]= {
	{"nodes", "nodes"},
	{"pins", "pins"},
	{"links", "links"},
	{"properties", "properties"},
	{"assetReferences", "assets"},
};

int64_t countArrayEntries(const configuru::Config& snapshot, const char* key)
{
	if (snapshot.has_key(key) && snapshot[key].is_array())
	{
		return (int64_t)snapshot[key].array_size();
	}

	return 0;
}
} // namespace

bool NodeGraphLogWriter::open(const std::filesystem::path& projectFilePath, const std::string& windowClassName)
{
	const std::filesystem::path logFolder= projectFilePath.parent_path() / "logs";
	if (!openLogFile(logFolder, "nodegraph"))
	{
		return false;
	}

	configuru::Config sessionLine= configuru::Config::object();
	sessionLine["event"]= "session";
	sessionLine["project"]= projectFilePath.string();
	sessionLine["window"]= windowClassName;
	sessionLine["started"]= makeLocalTimestampString("%Y-%m-%dT%H:%M:%S");
	writeLine(sessionLine);

	MIKAN_LOG_INFO("NodeGraphLogWriter") << "Node graph log: " << getLogFilePath().string();
	return true;
}

void NodeGraphLogWriter::writeBaseline(const std::filesystem::path& graphPath, const std::string& graphClassName,
									   const configuru::Config& snapshot)
{
	if (!isOpen())
		return;

	// Seed the delta tracking from the baseline counts
	makeCountDeltaDesc(snapshot);

	configuru::Config baselineLine= configuru::Config::object();
	baselineLine["event"]= "baseline";
	baselineLine["graph"]= graphPath.empty() ? std::string("untitled") : graphPath.string();
	baselineLine["class"]= graphClassName;
	baselineLine["t"]= nowEpochMs();
	baselineLine["snapshot"]= snapshot;
	writeLine(baselineLine);
}

void NodeGraphLogWriter::writeSnapshotCommit(int64_t sequenceNumber, const configuru::Config& snapshot)
{
	if (!isOpen())
		return;

	const std::string desc= makeCountDeltaDesc(snapshot);

	configuru::Config txnLine= configuru::Config::object();
	txnLine["event"]= "txn";
	txnLine["seq"]= sequenceNumber;
	txnLine["t"]= nowEpochMs();
	txnLine["desc"]= desc.empty() ? std::string("content change") : desc;
	txnLine["snapshot"]= snapshot;
	writeLine(txnLine);
}

void NodeGraphLogWriter::writeHistoryStep(const char* eventName, int steps, size_t cursor,
										  const configuru::Config& restoredSnapshot)
{
	if (!isOpen())
		return;

	// Track the restored counts so the next commit's delta is against them
	makeCountDeltaDesc(restoredSnapshot);

	configuru::Config stepLine= configuru::Config::object();
	stepLine["event"]= eventName;
	stepLine["steps"]= steps;
	stepLine["cursor"]= (int64_t)cursor;
	stepLine["t"]= nowEpochMs();
	writeLine(stepLine);
}

std::string NodeGraphLogWriter::makeCountDeltaDesc(const configuru::Config& snapshot)
{
	std::string desc;
	for (int i= 0; i < k_collectionCount; ++i)
	{
		const int64_t newCount= countArrayEntries(snapshot, k_snapshotCollections[i].jsonKey);
		const int64_t oldCount= m_lastCounts[i];
		m_lastCounts[i]= newCount;

		if (newCount != oldCount)
		{
			if (!desc.empty())
				desc+= ", ";
			desc+= std::string(k_snapshotCollections[i].label) + " " + std::to_string(oldCount) + "->"
				   + std::to_string(newCount);
		}
	}

	return desc;
}
