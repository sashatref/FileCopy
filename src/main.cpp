#include "project_pch.h"


bool process(const QString &_inputDir, const QString &_outputDir, const QStringList &_extList,
             const QStringList &_excludeFiles, const QStringList &_excludeDirs)
{
    QDir fromDir(_inputDir);

    const QSet<QString> excludeFileSet = _excludeFiles.toSet();
    const QSet<QString> excludeDirSet = _excludeDirs.toSet();

    QDir emptyDir;

    if(!emptyDir.mkpath(_outputDir))
    {
        qWarning() << QString("Error while create path [%1]").arg(_outputDir);
        return false;
    }

    QDirIterator it(_inputDir, _extList, QDir::Files, QDirIterator::Subdirectories);
    while(it.hasNext())
    {
        const QString filePath = it.next();
        QString inputRelPath = fromDir.relativeFilePath(filePath);
        inputRelPath.replace("\\", "/");

        bool skipFile = false;

        for(const QString &ex : excludeFileSet)
        {
            QRegExp rx(ex);
            rx.setPatternSyntax(QRegExp::Wildcard);
            if(rx.exactMatch(QFileInfo(inputRelPath).fileName()))
            {
                qDebug() << "Skip:" << inputRelPath;
                skipFile = true;
                break;
            }
        }

        if(skipFile) continue;

        QStringList folderTokens = QFileInfo(inputRelPath).path().split("/");
        for(const auto &tok : folderTokens)
        {
            if(excludeDirSet.contains(tok))
            {
                qDebug() << "Skip:" << inputRelPath;
                skipFile = true;
                break;
            }
        }

        if(skipFile) continue;

        const QString destPath = _outputDir + QDir::separator() + inputRelPath;

        QFileInfo destFileInfo(destPath);
        if(!emptyDir.mkpath(destFileInfo.absolutePath()))
        {
            qWarning() << QString("Error while create path [%1]").arg(destFileInfo.absolutePath());
            continue;
        }

        QFile outFile(destPath);
        if(outFile.exists())
        {
            if(!outFile.remove())
            {
                qWarning() << QString("Error while remove file [%1]").arg(outFile.fileName());
                continue;
            }
        }

        QFile inFile(filePath);
        if(!inFile.copy(filePath, destPath))
        {
            qWarning() << QString("Error while copy file [%1] to [%2] | %3")
                          .arg(filePath).arg(destPath).arg(inFile.errorString());
            continue;
        }
    }

    return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QElapsedTimer timer;
    timer.start();
    QCommandLineParser parser;

    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(QCommandLineOption(QStringList() << "i" << "input-dir", "input dir", "dir"));
    parser.addOption(QCommandLineOption(QStringList() << "o" << "output-dir", "Output dir", "dir"));
    parser.addOption(QCommandLineOption("ext", "Extensions", "ext"));
    parser.addOption(QCommandLineOption(QStringList() << "excludeFile" << "ef", "Exclude file", "wildcard"));
    parser.addOption(QCommandLineOption(QStringList() << "excludeDir" << "ed", "Exclude dir", "dirname"));

    parser.process(a);

    const QString inputDir = parser.value("i");
    const QString outputDir = parser.value("o");
    QStringList extList = parser.values("ext");
    if(extList.isEmpty())
    {
        extList = QStringList{ "*.h", "*.hpp" };
    }

    const QStringList excludeFiles = parser.values("excludeFile");
    const QStringList excludeDirs = parser.values("excludeDir");

    qInfo() << QString("Input dir [%1]").arg(inputDir);
    qInfo() << QString("Output dir [%2]").arg(outputDir);
    qInfo() << QString("Ext [%1]").arg(extList.join(", "));
    qInfo() << QString("Exclude file [%1]").arg(excludeFiles.join(", "));
    qInfo() << QString("Exclude dir [%1]").arg(excludeDirs.join(", "));

    if(inputDir.isEmpty())
    {
        qCritical() << "Input dir not specified";
        return 1;
    }

    if(outputDir.isEmpty())
    {
        qCritical() << "Output dir not specified";
        return 1;
    }

    if(!process(inputDir, outputDir, extList, excludeFiles, excludeDirs))
    {
        return -1;
    }
    qInfo() << QString("Finished in [%1 ms]").arg(timer.elapsed());

    return 0;
}
