#include "filenamerenderer.h"

#include "application.h"
#include "stringroutines.h"
#include "model.h"

#include <QtDebug>

const int dummyChar = 0x263c;

void FileNameRenderer::updateTags()
{DD;
    TagsRenderer::updateTags();
    if (m_pattern.isEmpty()) return;

    prefixes.clear();
    for (int i=0; i<m_newTags.size(); ++i) {
        QString s=m_newTags.at(i);

        //slashes in tags
        s.replace(QLS("/"),App->renameOptions.winCharsReplacer);

        //some operations we perform only on file names and not paths
        QStringList list=s.split(QChar(dummyChar));
        QString fileName=list.takeLast();

        if (App->renameOptions.replaceWinChars) {
            if (App->renameOptions.renamingOperation != 2)
                fileName = replaceWinChars(fileName, App->renameOptions.winCharsReplacer, true);
            if (App->renameOptions.applyToFolders) {
                for (int i=0; i<list.size(); ++i)
                    list[i] = replaceWinChars(list[i], App->renameOptions.winCharsReplacer, true);
            }
        }

        if (App->renameOptions.replaceSpaces) {
            if (App->renameOptions.renamingOperation != 2) {
                fileName.replace(QLS(" "),App->renameOptions.spacesReplacer);
                fileName.replace(QLS("\t"),App->renameOptions.spacesReplacer);
            }
            if (App->renameOptions.applyToFolders) {
                list.replaceInStrings(QLS(" "),App->renameOptions.spacesReplacer);
                list.replaceInStrings(QLS("\t"),App->renameOptions.spacesReplacer);
            }
        }

        if (App->renameOptions.removeDiacritics) {
            if (App->renameOptions.renamingOperation != 2)
                fileName = removeDiacritics(fileName);
            if (App->renameOptions.applyToFolders) {
                for (int i=0; i<list.size(); ++i)
                    list[i] = removeDiacritics(list[i]);
            }
        }

        if (App->renameOptions.changeCase) {
            if (App->renameOptions.renamingOperation != 2)
                fileName = changeCase(fileName, (Case::Case)App->renameOptions.ccase);
            if (App->renameOptions.applyToFolders) {
                for (int i=0; i<list.size(); ++i)
                    list[i] = changeCase(list[i], (Case::Case)App->renameOptions.ccase);
            }
        }

        QFileInfo fi(m->fileAtSelection(i).fullFileName());
        int suffixLength = fi.suffix().length()+1;

        // always sanitize file name in Windows
#ifdef Q_OS_WIN
        if (fileName.length()+suffixLength > MAXIMUM_FILENAME_LENGTH)
            fileName.truncate(MAXIMUM_FILENAME_LENGTH - suffixLength);
#endif

        if (App->renameOptions.renamingOperation != 2) {
            if (App->renameOptions.trimFileLength
                && App->renameOptions.maximumLength - suffixLength > 0) {
                fileName.truncate(App->renameOptions.maximumLength-suffixLength);
            }
        }
        QString pathPrefix = App->renameOptions.destinationFolder;
        if (pathPrefix.isEmpty())
            pathPrefix = fi.canonicalPath();

        if (App->renameOptions.renamingOperation == 2) {
            QDir d = fi.dir();
            d.cdUp();
            pathPrefix = d.canonicalPath();
        }

        prefixes << pathPrefix;
        list.prepend(pathPrefix);
        QString newFileName = fileName + "." + fi.suffix();

        list.append(newFileName);
        m_newTags[i] = list.join(QSL("/"));
    }
}

void FileNameRenderer::preprocess()
{DD;
    TagsRenderer::preprocess();

    if (App->renameOptions.renamingOperation == 2) {
        // no internal folders allowed in folder renaming mode
        m_pattern.replace(QLS("\\"), App->renameOptions.winCharsReplacer);
        m_pattern.replace(QLS("/"), App->renameOptions.winCharsReplacer);
        m_pattern.append("/%f");
    }

    m_pattern.replace(QLS("\\"),QLS("/"));
    m_pattern.replace(QLS("/"),QString(QChar(dummyChar)));
}
