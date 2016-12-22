#ifndef CC_COMMAND_LINE_INTERFACE_HEADER
#define CC_COMMAND_LINE_INTERFACE_HEADER

//qCC_db
#include <ccGenericMesh.h>
#include <ccPointCloud.h>

//Qt
#include <QString>
#include <QStringList>
#include <QSharedPointer>
#include <QDir>

//System
#include <vector>

class ccProgressDialog;
class QDialog;

//! Loaded entity description
struct CLEntityDesc
{
	QString basename;
	QString path;
	int indexInFile;

	CLEntityDesc(QString filename, int _indexInFile = -1)
		: indexInFile(_indexInFile)
	{
		if (filename.isNull())
		{
			basename = "unknown";
			path = QDir::currentPath();
		}
		else
		{
			QFileInfo fi(filename);
			basename = fi.completeBaseName();
			path = fi.path();
		}
	}
	
	CLEntityDesc(QString _basename, QString _path, int _indexInFile = -1)
		: basename(_basename)
		, path(_path)
		, indexInFile(_indexInFile)
	{
	}
	
	virtual ccHObject* getEntity() = 0;
};

//! Loaded group description
struct CLGroupDesc : CLEntityDesc
{
	ccHObject* groupEntity;

	CLGroupDesc(ccHObject* group,
				QString basename,
				QString path = QString())
		: CLEntityDesc(basename, path)
		, groupEntity(group)
	{}

	virtual ccHObject* getEntity() { return groupEntity; }
};

//! Loaded cloud description
struct CLCloudDesc : CLEntityDesc
{
	ccPointCloud* pc;

	CLCloudDesc() : CLEntityDesc("Unnamed cloud") {}

	CLCloudDesc(ccPointCloud* cloud,
				QString filename = QString(),
				int index = -1)
		: CLEntityDesc(filename, index)
		, pc(cloud)
	{}

	CLCloudDesc(ccPointCloud* cloud,
				QString basename,
				QString path,
				int index = -1)
		: CLEntityDesc(basename, path, index)
		, pc(cloud)
	{}

	virtual ccHObject* getEntity() { return static_cast<ccHObject*>(pc); }
};

//! Loaded mesh description
struct CLMeshDesc : CLEntityDesc
{
	ccGenericMesh* mesh;

	CLMeshDesc() : CLEntityDesc("Unnamed mesh") {}

	CLMeshDesc(	ccGenericMesh* _mesh,
				QString filename = QString(),
				int index = -1)
		: CLEntityDesc(filename, index)
		, mesh(_mesh)
	{}

	CLMeshDesc(	ccGenericMesh* _mesh,
				QString basename,
				QString path,
				int index = -1)
		: CLEntityDesc(basename, path, index)
		, mesh(_mesh)
	{}

	virtual ccHObject* getEntity() { return static_cast<ccHObject*>(mesh); }
};

//! Command line interface
class ccCommandLineInterface
{
public: //constructor

	//! Default constructor
	ccCommandLineInterface()
		: m_orphans("orphans")
		, m_silentMode(false)
		, m_autoSaveMode(true)
		, m_addTimestamp(true)
		, m_precision(12)
	{}

public: //commands

	//! Generic command interface
	struct Command
	{
		//! Shared type
		typedef QSharedPointer<Command> Shared;

		//! Default constructor
		Command(QString _name, QString _keyword)
			: name(_name)
			, keyword(_keyword)
		{}

		//! Main process
		virtual bool process(ccCommandLineInterface& cmd) = 0;

		//! Command name
		QString name;
		//! Command keyword
		QString keyword;
	};

public: //virtual methods

	//! Registers a new command
	/** \return success
	**/
	virtual bool registerCommand(Command::Shared command) = 0;

	//! Exports a cloud or a mesh
	/** \return error string (if any)
	**/
	virtual QString exportEntity(	CLEntityDesc& entityDesc,
									QString suffix = QString(),
									QString* outputFilename = 0,
									bool forceIsCloud = false,
									bool forceNoTimestamp = false) = 0;

	//! Saves all clouds
	/** \param suffix optional suffix
		\param allAtOnce whether to save all clouds in the same file or one cloud per file
		\return success
	**/
	virtual bool saveClouds(QString suffix = QString(), bool allAtOnce = false) = 0;

	//! Saves all meshes
	/** \param suffix optional suffix
		\param allAtOnce whether to save all meshes in the same file or one mesh per file
		\return success
	**/
	virtual bool saveMeshes(QString suffix = QString(), bool allAtOnce = false) = 0;

	//! Removes all clouds (or only the last one ;)
	virtual void removeClouds(bool onlyLast = false) = 0;

	//! Removes all meshes (or only the last one ;)
	virtual void removeMeshes(bool onlyLast = false) = 0;

	//! Returns the list of arguments
	virtual QStringList& arguments() = 0;
	//! Returns the list of arguments (const version)
	virtual const QStringList& arguments() const = 0;

	//! Returns a (shared) progress dialog (if any is available)
	virtual ccProgressDialog* progressDialog() { return 0; }
	//! Returns a (widget) parent (if any is available)
	virtual QDialog* widgetParent() { return 0; }

	//logging
	virtual void print(const QString& message) const = 0;
	virtual void warning(const QString& message) const = 0;
	virtual bool error(const QString& message) const = 0; //must always return false!

public: //access to data

	//! Currently opened point clouds and their filename
	virtual std::vector< CLCloudDesc >& clouds() { return m_clouds; }
	//! Currently opened point clouds and their filename (const version)
	virtual const std::vector< CLCloudDesc >& clouds() const { return m_clouds; }

	//! Currently opened meshes and their filename
	virtual std::vector< CLMeshDesc >& meshes() { return m_meshes; }
	//! Currently opened meshes and their filename (const version)
	virtual const std::vector< CLMeshDesc >& meshes() const { return m_meshes; }

	//! Returns the list of orhpan entities (neither clouds nor meshes)
	virtual ccHObject& orphans() { return m_orphans; }
	//! Returns the list of orhpan entities (neither clouds nor meshes) (const version)
	virtual const ccHObject& orphans() const { return m_orphans; }

	//! Toggles silent mode
	/** Must be called BEFORE calling start.
	**/
	void toggleSilentMode(bool state) { m_silentMode = state; }
	//! Returns the silent mode
	bool silentMode() const { return m_silentMode; }

	//! Sets whether files should be automatically saved (after each process) or not
	void toggleAutoSaveMode(bool state) { m_autoSaveMode = state; }
	//! Returns whether files should be automatically saved (after each process) or not
	bool autoSaveMode() const { return m_autoSaveMode; }

	//! Sets whether a timestamp should be automatically added to output files or not
	void toggleAddTimestamp(bool state) { m_addTimestamp = state; }
	//! Returns whether a timestamp should be automatically added to output files or not
	bool addTimestamp() const { return m_addTimestamp; }

	//! Sets the numerical precision
	void setNumericalPrecision(int p) { m_precision = p; }
	//! Returns the numerical precision
	int numericalPrecision() const { return m_precision; }

protected: //members

	//! Currently opened point clouds and their filename
	std::vector< CLCloudDesc > m_clouds;

	//! Currently opened meshes and their filename
	std::vector< CLMeshDesc > m_meshes;

	//! Oprhan entities
	ccHObject m_orphans;

	//! Silent mode
	bool m_silentMode;

	//! Whether files should be automatically saved (after each process) or not
	bool m_autoSaveMode;

	//! Whether a timestamp should be automatically added to output files or not
	bool m_addTimestamp;

	//! Default numerical precision for ASCII output
	int m_precision;

};

#endif //CC_COMMAND_LINE_INTERFACE_HEADER