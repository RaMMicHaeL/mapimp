/*

  .....____    ...::$$$$=$=$=$  ...____
_____/    /__        $$     $$___/    /___
\           //\\____$$   __$$\__         /
 \__     __//        /  / $$__  /    ___/
   /    /___\   \.   ) / $$  / /    /___
  /        /    /   / / $$  __/        /
  \_______....  ___/ / $$_   \\_______/
          /    /$$=$=$=$/ \   \
      ...(____/           /   /
                 .......:::./

    mapimp plugin version 1.0
    coded by takerZ
    tPORt, 2009-2012

YOU CAN USE THE SOURCE ANY WAY YOU LIKE WITHOUT
RESTRICTIONS, BUT STILL REMEMBER THAT THIS SOFTWARE
IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE BLAH BLAH BLAH
*/

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>

#include "mapimp.h"

#ifndef MASKTEST

HWND      g_hwndOlly,
          g_hwndMaskList;
HINSTANCE g_hInstance,
          g_hInstDLL;
HFONT     g_hFont;
BOOL      g_SessionStarted;
BOOL      g_Autoloaded;

#endif

pconfig_t g_Config;

#ifndef MASKTEST

static TCHAR c_About[] =
    "mapimp plugin v1.0\n"
    "coded by takerZ\n\n"
    "Thanks to all those, who had contributed\n"
    "their code, ideas and bugreports:\n\n"
    "9999 (mysterious guy from the board)\n"
    "awerto\n"
    "BoRoV\n"
    "DillerInc\n"
    "Jupiter\n"
    "PE_Kill\n"
    "RaMMicHaeL\n"
    "Runner\n"
    "sendersu\n"
    "void\n\n"
    "My excuses if forgot somebody.\n\n"
    "tPORt, 2009-2012";

static ACCEL c_AccelTable[] =
{
    {FALT     | FVIRTKEY, 'C',       ID_COMMENTS},
    {FALT     | FVIRTKEY, 'L',       ID_LABELS},
    {FALT     | FVIRTKEY, 'S',       ID_SKIP},
    {FALT     | FVIRTKEY, 'O',       ID_OVERWRITE},
    {FALT     | FVIRTKEY, 'F',       ID_FILE},
    {FALT     | FVIRTKEY, 'M',       ID_MEMORY},
    {FALT     | FVIRTKEY, 'A',       ID_ASKTOIMPORT},
    {FALT     | FVIRTKEY, 'I',       ID_IMPORTALWAYS},
    {FALT     | FVIRTKEY, 'N',       ID_DONOTHING},
    {FALT     | FVIRTKEY, 'U',       ID_USEMASKS},
    {FALT     | FVIRTKEY, 'E',       ID_DEMANGLE},

    {FCONTROL | FVIRTKEY, 'A',       ID_ADD},
    {FCONTROL | FVIRTKEY, 'I',       ID_INSERT},
    {FCONTROL | FVIRTKEY, 'E',       ID_EDIT},
    {           FVIRTKEY, VK_DELETE, ID_DELETE},
    {FCONTROL | FVIRTKEY, 'S',       ID_SAVE},
    {FCONTROL | FVIRTKEY, 'L',       ID_LOAD},
    {FCONTROL | FVIRTKEY, VK_UP,     ID_INCREASE},
    {FCONTROL | FVIRTKEY, VK_DOWN,   ID_DECREASE},
    {FCONTROL | FVIRTKEY, VK_PRIOR,  ID_MAXINCREASE},
    {FCONTROL | FVIRTKEY, VK_NEXT,   ID_MAXDECREASE},

    {           FVIRTKEY, VK_RETURN, ID_APPLY},
    {           FVIRTKEY, VK_ESCAPE, ID_CANCEL}
};

#endif

int mask_compile(pmask_t msk, TCHAR* subj)
{
    int   mod = 0,
          cntr_msk = 1,
          cntr_repl;
    TCHAR buffer[TEXTLEN];

    if (subj[0] == '/')
    {
        if ((subj[cntr_msk] == 'i') || (subj[cntr_msk] == 'I'))
        {
            mod = PCRE_CASELESS;
            cntr_msk++;
        }

        if ((subj[cntr_msk] == 's') || (subj[cntr_msk] == 'S'))
        {
            msk->regex = pcre_compile2(&subj[cntr_msk + 1], mod, &msk->errcode, &msk->errptr, &msk->erroffset, NULL);

            if (!msk->errcode)
            {
                msk->extra = pcre_study((const pcre*)msk->regex, 0, &msk->errptr);
                msk->type = FILTER_SKIP;
            }
            else
            {
                msk->erroffset += cntr_msk + 1;
            }
        }
        else if ((subj[cntr_msk] == 'c') || (subj[cntr_msk] == 'C'))
        {
            msk->regex = pcre_compile2(&subj[cntr_msk + 1], mod, &msk->errcode, &msk->errptr, &msk->erroffset, NULL);

            if (!msk->errcode)
            {
                msk->extra = pcre_study(msk->regex, 0, &msk->errptr);
                msk->type = FILTER_CUT;
            }
            else
            {
                msk->erroffset += cntr_msk + 1;
            }
        }
        else if ((subj[cntr_msk] == 'r') || (subj[cntr_msk] == 'R'))
        {
            msk->insert = -1;

            for (cntr_msk++, cntr_repl = 0; TRUE; cntr_msk++)
            {
                if (subj[cntr_msk] == '\0')
                {
                    msk->errcode = FILTER_INVALID_REPLACEMENT;

                    return FILTER_INVALID_REPLACEMENT;
                }
                else if (subj[cntr_msk] == '/')
                {
                    break;
                }
                else if (subj[cntr_msk] == '\\')
                {
                    buffer[cntr_repl] = subj[cntr_msk + 1];
                    cntr_msk++;
                    cntr_repl++;
                }
                else if (subj[cntr_msk] == '%')
                {
                    if (msk->insert < 0)
                    {
                        msk->insert = cntr_repl;
                    }
                }
                else
                {
                    buffer[cntr_repl] = subj[cntr_msk];
                    cntr_repl++;
                }
            }
            buffer[cntr_repl] = '\0';

            msk->regex = pcre_compile2(&subj[cntr_msk + 1], mod, &msk->errcode, &msk->errptr, &msk->erroffset, NULL);

            if (!msk->errcode)
            {
                msk->repl_s = (strlen(buffer) + 1) * sizeof(TCHAR);
                msk->repl = malloc(msk->repl_s);
                strcpy(msk->repl, buffer);
                msk->extra = pcre_study(msk->regex, 0, &msk->errptr);
                msk->type = FILTER_REPLACE;
            }
            else
            {
                msk->erroffset += cntr_msk + 1;
            }
        }
        else
        {
            msk->errcode = FILTER_INVALID_KEY;
            msk->erroffset = cntr_msk + 1;
        }
    }
    else
    {
        msk->errcode = FILTER_INVALID_MASK;
        msk->erroffset = 1;
    }

    return msk->errcode;
}

void mask_error(pmask_t msk, TCHAR* errbuf)
{
    switch (msk->errcode)
    {
        case FILTER_NOERROR:
            break;

        case FILTER_INVALID_MASK:
            sprintf(errbuf, "missing mask magic / at offset %d", msk->erroffset);
            break;

        case FILTER_INVALID_KEY:
            sprintf(errbuf, "invalid key at offset %d", msk->erroffset);
            break;

        case FILTER_INVALID_REPLACEMENT:
            strcpy(errbuf, "missing replacement terminating /");
            break;

        default:
            sprintf(errbuf, "%s at offset %d", msk->errptr, msk->erroffset);
            break;
    }
}

int mask_filter(pname_t name)
{
    int     offset,
            result = FILTER_NOTHING;
    size_t  rm_ln,
            repl_ln,
            dest,
            replrest_ln,
            name_ln = name->size / sizeof(TCHAR) - 1;
    TCHAR*  buffer;
    match_t match;
    pmask_t msk;

    for (msk = (pmask_t)g_Config->masks->first; msk; msk = msk->next)
    {
        if (msk->type == FILTER_SKIP)
        {
            /* if pcre_exec() fails, it returns negative values */
            if (pcre_exec((const pcre*)msk->regex,
                          (const pcre_extra*)msk->extra,
                          name->buffer,
                          name_ln,
                          0,
                          PCRE_NOTEMPTY,
                          NULL,
                          0) >= 0)
            {
                name->buffer[0] = '\0';
                name_ln = 0;
                result = FILTER_SKIP;

                break;
            }
        }
        else if (msk->type == FILTER_CUT)
        {
            while (pcre_exec((const pcre*)msk->regex,
                             (const pcre_extra*)msk->extra,
                             name->buffer,
                             name_ln,
                             0,
                             PCRE_NOTEMPTY,
                             (int*)&match,
                             OVECTOR_COMP_COUNT) >= 0)
            {
                memcpy(&name->buffer[match.rm_so], &name->buffer[match.rm_eo], (name_ln - match.rm_eo + 1) * sizeof(TCHAR));
                name_ln -= match.rm_eo - match.rm_so;
                result = FILTER_CUT;
            }
        }
        else /* if (msk->type == FILTER_REPLACE) */
        {
            repl_ln = msk->repl_s / sizeof(TCHAR) - 1;
            offset = -(int)repl_ln;

            while (pcre_exec((const pcre*)msk->regex,
                             (const pcre_extra*)msk->extra,
                             name->buffer,
                             name_ln,
                             repl_ln + offset,
                             PCRE_NOTEMPTY | PCRE_NO_UTF8_CHECK,
                             (int*)&match,
                             OVECTOR_COMP_COUNT) >= 0)
            {
                rm_ln = match.rm_eo - match.rm_so;

                if (msk->insert >= 0)
                {
                    buffer = malloc((name_ln + repl_ln + 1) * sizeof(TCHAR));
                    memcpy(buffer, name->buffer, match.rm_so * sizeof(TCHAR));
                    memcpy(&buffer[dest = match.rm_so], msk->repl, msk->insert * sizeof(TCHAR));
                    memcpy(&buffer[dest += msk->insert], &name->buffer[match.rm_so], rm_ln * sizeof(TCHAR));
                    memcpy(&buffer[dest += rm_ln], &msk->repl[msk->insert], (replrest_ln = repl_ln - msk->insert) * sizeof(TCHAR));
                    memcpy(&buffer[dest + replrest_ln], &name->buffer[match.rm_eo], (name_ln - match.rm_eo + 1) * sizeof(TCHAR));
                    free(name->buffer);
                    name->buffer = buffer;
                    offset = (int)match.rm_eo;
                    name_ln += repl_ln;
                }
                else if (repl_ln > rm_ln)
                {
                    buffer = malloc((name_ln + (repl_ln - rm_ln) + 1) * sizeof(TCHAR));
                    memcpy(buffer, name->buffer, match.rm_so * sizeof(TCHAR));
                    memcpy(&buffer[match.rm_so], msk->repl, msk->repl_s - sizeof(TCHAR));
                    memcpy(&buffer[match.rm_so + repl_ln], &name->buffer[match.rm_eo], (name_ln - match.rm_eo + 1) * sizeof(TCHAR));
                    free(name->buffer);
                    name->buffer = buffer;
                    offset = (int)match.rm_so;
                    name_ln += repl_ln - rm_ln;
                }
                else
                {
                    memcpy(&name->buffer[match.rm_so], msk->repl, msk->repl_s - sizeof(TCHAR));
                    memcpy(&name->buffer[match.rm_so + repl_ln], &name->buffer[match.rm_eo], (name_ln - match.rm_eo + 1) * sizeof(TCHAR));
                    offset = (int)match.rm_so;
                    name_ln -= rm_ln - repl_ln;
                }

                result = FILTER_REPLACE;
            }
        }
    }
    name->size = (name_ln + 1) * sizeof(TCHAR);

    return result;
}

plist_t list_create()
{
    plist_t list = malloc(sizeof(list_t));

    list->count = 0;
    list->first = NULL;
    list->last = NULL;

    return list;
}

pmask_t list_add_mask(plist_t lst, TCHAR* str)
{
    pmask_t msk = malloc(sizeof(mask_t));

    if (str)
    {
        if (!mask_compile(msk, str))
        {
            msk->buffer = malloc((strlen(str) + 1) * sizeof(TCHAR));
            strcpy(msk->buffer, str);

            if (!lst->first)
            {
                lst->first = (LPVOID)msk;
                lst->last = (LPVOID)msk;
            }
            else
            {
                ((pmask_t)lst->last)->next = msk;
                lst->last = (LPVOID)msk;
            }

            msk->next = NULL;
            lst->count++;
        }
    }

    return msk;
}

plist_t list_add_name(plist_t lst, TCHAR* name, size_t length, ULONG segment, ULONG offset)
{
    pname_t nm = malloc(sizeof(name_t));

    if (name)
    {
        nm->buffer = malloc(nm->size = (length + 1) * sizeof(TCHAR));
        strcpy(nm->buffer, name);
    }
    else
    {
        nm->size = 0;
        nm->buffer = NULL;
    }

    nm->segment = segment;
    nm->offset = offset;

    if (!lst->first)
    {
        lst->first = (LPVOID)nm;
        lst->last = (LPVOID)nm;
    }
    else
    {
        ((pname_t)lst->last)->next = nm;
        lst->last = (LPVOID)nm;
    }

    nm->next = NULL;
    lst->count++;

    return lst;
}

void list_free_masks(plist_t lst)
{
    pmask_t msk = (pmask_t)lst->first,
            lastmsk;

    while (msk)
    {
        pcre_free(msk->regex);

        if (msk->extra)
        {
            pcre_free(msk->extra);
        }

        free(msk->buffer);

        if (msk->type == FILTER_REPLACE)
        {
            free(msk->repl);
        }

        lastmsk = msk;
        msk = msk->next;

        free(lastmsk);
    }

    free(lst);
}

void list_free_names(plist_t lst)
{
    pname_t nm = (pname_t)lst->first,
            lastnm;

    while (nm)
    {
        if (nm->buffer)
        {
            free(nm->buffer);
        }

        lastnm = nm;
        nm = nm->next;

        free(lastnm);
    }

    free(lst);
}

#ifndef MASKTEST

BOOL config_create(TCHAR* path, pconfig_t conf)
{
    BOOL    result = FALSE;
    FILE*   file = fopen(path, "w");
    pmask_t msk;

    if (file)
    {
        fprintf(file, "# WARNING! If you want to edit this config file manually,\n"
                      "# please read corresponding comments and be careful\n"
                      "# Import comments (0 - No; 1 - Yes)\n"
                      "%s=%d\n"
                      "# Import labels (0 - No; 1 - Yes)\n"
                      "%s=%d\n"
                      "# Check for collisions (0 - Overwrite; 1 - Skip if collision)\n"
                      "%s=%d\n"
                      "# Read segments from (0 - Read only from memory; 1 - Read from file first, then\n"
                      "# from memory if failed)\n"
                      "%s=%d\n"
                      "# Autoimport map if present (0 - Disabled; 1 - Ask to import;\n"
                      "# 2 - Import always)\n"
                      "%s=%d\n"
                      "# Demangle names (0 - No; 1 - Yes)\n"
                      "%s=%d\n"
                      "# Filter names using masks (0 - No; 1 - Yes)\n"
                      "%s=%d\n"
                      "# User specified masks. The \"value\" specifies the number of masks\n"
                      "# next \"value\" lines are masks themselves. If the mask fails to compile it is\n"
                      "# ignored\n"
                      "%s=%d\n",
                      CONFIG_STR_COMMENTS, conf->comments,
                      CONFIG_STR_LABELS, conf->labels,
                      CONFIG_STR_COLLISIONS, conf->check_collisions,
                      CONFIG_STR_FILESEG, conf->read_file_segments,
                      CONFIG_STR_AUTOIMPORT, conf->autoimport,
                      CONFIG_STR_DEMANGLE, conf->demangle,
                      CONFIG_STR_USEMASKS, conf->use_masks,
                      CONFIG_STR_MASKS, conf->masks->count);

        msk = (pmask_t)conf->masks->first;

        while (msk)
        {
            fprintf(file, "%s\n", msk->buffer);
            msk = msk->next;
        }

        fclose(file);

        result = TRUE;
    }

    return result;
}

TCHAR* config_locate(TCHAR* buffer)
{
    GetModuleFileName(g_hInstDLL, buffer, MAX_PATH);
    strcpy(strrchr(buffer, '\\'), "\\mapimp.cfg");

    return buffer;
}

void config_defaults(pconfig_t conf)
{
    conf->comments           = TRUE;
    conf->labels             = TRUE;
    conf->autoimport         = AUTOIMPORT_ASK;
    conf->read_file_segments = FALSE;
    conf->check_collisions   = TRUE;
    conf->demangle           = TRUE;
    conf->use_masks          = FALSE;    
    conf->masks              = list_create();
}

pconfig_t config_parse(TCHAR* path)
{
    FILE*     file = fopen(path, "r");
    TCHAR     buffer[TEXTLEN],
              param[CONFIG_STR_MAXSIZE];
    size_t    len,
              count;
    pmask_t   msk;
    pconfig_t conf = malloc(sizeof(config_t));

    config_defaults(conf);

    if (file)
    {
        while (fgets(buffer, TEXTLEN, file))
        {
            len = sizeof(CONFIG_STR_COMMENTS) - 1;
            strncpy(param, buffer, len);
            param[len] = '\0';

            if (!stricmp(CONFIG_STR_COMMENTS, param))
            {
                strtok(buffer, CONFIG_SEPARATOR_CHARSET);
                conf->comments = atoi(strtok(NULL, CONFIG_SEPARATOR_CHARSET));
                continue;
            }

            len = sizeof(CONFIG_STR_LABELS) - 1;
            strncpy(param, buffer, len);
            param[len] = '\0';

            if (!stricmp(CONFIG_STR_LABELS, param))
            {
                strtok(buffer, CONFIG_SEPARATOR_CHARSET);
                conf->labels = atoi(strtok(NULL, CONFIG_SEPARATOR_CHARSET));
                continue;
            }

            len = sizeof(CONFIG_STR_COLLISIONS) - 1;
            strncpy(param, buffer, len);
            param[len] = '\0';

            if (!stricmp(CONFIG_STR_COLLISIONS, param))
            {
                strtok(buffer, CONFIG_SEPARATOR_CHARSET);
                conf->check_collisions = atoi(strtok(NULL, CONFIG_SEPARATOR_CHARSET));
                continue;
            }

            len = sizeof(CONFIG_STR_FILESEG) - 1;
            strncpy(param, buffer, len);
            param[len] = '\0';

            if (!stricmp(CONFIG_STR_FILESEG, param))
            {
                strtok(buffer, CONFIG_SEPARATOR_CHARSET);
                conf->read_file_segments = atoi(strtok(NULL, CONFIG_SEPARATOR_CHARSET));
                continue;
            }

            len = sizeof(CONFIG_STR_AUTOIMPORT) - 1;
            strncpy(param, buffer, len);
            param[len] = '\0';

            if (!stricmp(CONFIG_STR_AUTOIMPORT, param))
            {
                strtok(buffer, CONFIG_SEPARATOR_CHARSET);
                conf->autoimport = atoi(strtok(NULL, CONFIG_SEPARATOR_CHARSET));
                continue;
            }

            len = sizeof(CONFIG_STR_DEMANGLE) - 1;
            strncpy(param, buffer, len);
            param[len] = '\0';

            if (!stricmp(CONFIG_STR_DEMANGLE, param))
            {
                strtok(buffer, CONFIG_SEPARATOR_CHARSET);
                conf->demangle = atoi(strtok(NULL, CONFIG_SEPARATOR_CHARSET));
                continue;
            }

            len = sizeof(CONFIG_STR_USEMASKS) - 1;
            strncpy(param, buffer, len);
            param[len] = '\0';

            if (!stricmp(CONFIG_STR_USEMASKS, param))
            {
                strtok(buffer, CONFIG_SEPARATOR_CHARSET);
                conf->use_masks = atoi(strtok(NULL, CONFIG_SEPARATOR_CHARSET));
                continue;
            }

            len = sizeof(CONFIG_STR_MASKS) - 1;
            strncpy(param, buffer, len);
            param[len] = '\0';

            if (!stricmp(CONFIG_STR_MASKS, param))
            {
                strtok(buffer, CONFIG_SEPARATOR_CHARSET);
                conf->masks = list_create();
                count = atoi(strtok(NULL, CONFIG_SEPARATOR_CHARSET));

                while (conf->masks->count < count && fgets(buffer, TEXTLEN, file))
                {
                    msk = list_add_mask(conf->masks, strtok(buffer, "\n"));

                    if (msk->errcode)
                    {
                        free(msk);
                    }
                }
            }
        }

        fclose(file);
    }

    return conf;
}

BOOL menuitem_add(HMENU hmenu, TCHAR* lpstr, UINT index, UINT id)
{
    MENUITEMINFO mi;

    mi.cbSize = sizeof(MENUITEMINFO);
    mi.fMask = MIIM_STATE | MIIM_TYPE | MIIM_ID;

    if (lpstr)
    {
        mi.fType = MFT_STRING;
        mi.dwTypeData = lpstr;
        mi.cch = strlen(lpstr);
    }
    else
    {
        mi.fType = MFT_SEPARATOR;
    }

    mi.fState = MFS_ENABLED;
    mi.wID = id;

    return InsertMenuItem(hmenu, index, FALSE, &mi);
}

LRESULT CALLBACK listbox_msgproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    HMENU   pm;
    POINT   point;
    LRESULT result;

    switch (msg)
    {
        case WM_RBUTTONDOWN:
            pm = CreatePopupMenu();

            menuitem_add(pm, "Add", 0, ID_ADD);

            if (SendMessage(hwnd, LB_GETCURSEL, 0, 0) != LB_ERR)
            {
                menuitem_add(pm, "Insert", 1, ID_INSERT);
                menuitem_add(pm, "Edit", 2, ID_EDIT);
                menuitem_add(pm, "Delete", 3, ID_DELETE);
            }

            menuitem_add(pm, NULL, 4, 0);

            if (SendMessage(hwnd, LB_GETCOUNT, 0, 0) > 0)
            {
                menuitem_add(pm, "Save list", 5, ID_SAVE);
            }

            menuitem_add(pm, "Load list", 6, ID_LOAD);

            point.x = LOWORD(lparam);
            point.y = HIWORD(lparam);
            ClientToScreen(hwnd, &point);
            TrackPopupMenu(pm, TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x,
                point.y, 0, GetParent(hwnd), NULL);

            DestroyMenu(pm);

            result = 0;
            break;

        case WM_LBUTTONDBLCLK:
            result = SendMessage(GetParent(hwnd), WM_COMMAND, ID_EDIT, lparam);
            break;

        default:
            result = CallWindowProc((WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA), hwnd, msg, wparam, lparam);
            break;
    }

    return result;
}

LRESULT CALLBACK configwnd_msgproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    HWND    wnd;
    RECT    rect;
    FILE*   file;
    TCHAR   buffer[TEXTLEN],
            errbuf[ERRBUFLEN];
    pmask_t msk;
    mask_t  msk_tmp;
    int     counter,
            index,
            height,
            count,
            result;

    switch (msg)
    {
        case WM_ACTIVATE:
            SetFocus(g_hwndMaskList);
            break;

        case WM_COMMAND:
            switch (LOWORD(wparam))
            {
                case ID_COMMENTS:
                    if (HIWORD(wparam) == 1)
                    {
                        CheckDlgButton(hwnd, ID_COMMENTS, IsDlgButtonChecked(hwnd, ID_COMMENTS) ^ BST_CHECKED);
                    }
                    break;

                case ID_LABELS:
                    if (HIWORD(wparam) == 1)
                    {
                        CheckDlgButton(hwnd, ID_LABELS, IsDlgButtonChecked(hwnd, ID_LABELS) ^ BST_CHECKED);
                    }
                    break;

                case ID_USEMASKS:
                    if (HIWORD(wparam) == 1)
                    {
                        CheckDlgButton(hwnd, ID_USEMASKS, IsDlgButtonChecked(hwnd, ID_USEMASKS) ^ BST_CHECKED);
                    }
                    break;

                case ID_DEMANGLE:
                    if (HIWORD(wparam) == 1)
                    {
                        CheckDlgButton(hwnd, ID_DEMANGLE, IsDlgButtonChecked(hwnd, ID_DEMANGLE) ^ BST_CHECKED);
                    }
                    break;

                case ID_SKIP:
                    CheckDlgButton(hwnd, ID_SKIP, BST_CHECKED);
                    CheckDlgButton(hwnd, ID_OVERWRITE, BST_UNCHECKED);
                    break;

                case ID_OVERWRITE:
                    CheckDlgButton(hwnd, ID_SKIP, BST_UNCHECKED);
                    CheckDlgButton(hwnd, ID_OVERWRITE, BST_CHECKED);
                    break;

                case ID_FILE:
                    CheckDlgButton(hwnd, ID_FILE, BST_CHECKED);
                    CheckDlgButton(hwnd, ID_MEMORY, BST_UNCHECKED);
                    break;

                case ID_MEMORY:
                    CheckDlgButton(hwnd, ID_MEMORY, BST_CHECKED);
                    CheckDlgButton(hwnd, ID_FILE, BST_UNCHECKED);
                    break;

                case ID_ASKTOIMPORT:
                    CheckDlgButton(hwnd, ID_ASKTOIMPORT, BST_CHECKED);
                    CheckDlgButton(hwnd, ID_IMPORTALWAYS, BST_UNCHECKED);
                    CheckDlgButton(hwnd, ID_DONOTHING, BST_UNCHECKED);
                    break;

                case ID_IMPORTALWAYS:
                    CheckDlgButton(hwnd, ID_ASKTOIMPORT, BST_UNCHECKED);
                    CheckDlgButton(hwnd, ID_IMPORTALWAYS, BST_CHECKED);
                    CheckDlgButton(hwnd, ID_DONOTHING, BST_UNCHECKED);
                    break;

                case ID_DONOTHING:
                    CheckDlgButton(hwnd, ID_ASKTOIMPORT, BST_UNCHECKED);
                    CheckDlgButton(hwnd, ID_IMPORTALWAYS, BST_UNCHECKED);
                    CheckDlgButton(hwnd, ID_DONOTHING, BST_CHECKED);
                    break;

                case ID_CANCEL:
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                    break;

                case ID_APPLY:
                    if (IsDlgButtonChecked(hwnd, ID_COMMENTS) == BST_CHECKED)
                    {
                        g_Config->comments = TRUE;
                    }
                    else
                    {
                        g_Config->comments = FALSE;
                    }

                    if (IsDlgButtonChecked(hwnd, ID_LABELS) == BST_CHECKED)
                    {
                        g_Config->labels = TRUE;
                    }
                    else
                    {
                        g_Config->labels = FALSE;
                    }

                    if (IsDlgButtonChecked(hwnd, ID_SKIP) == BST_CHECKED)
                    {
                        g_Config->check_collisions = TRUE;
                    }
                    else
                    {
                        g_Config->check_collisions = FALSE;
                    }

                    if (IsDlgButtonChecked(hwnd, ID_FILE) == BST_CHECKED)
                    {
                        g_Config->read_file_segments = TRUE;
                    }
                    else
                    {
                        g_Config->read_file_segments = FALSE;
                    }

                    if (IsDlgButtonChecked(hwnd, ID_ASKTOIMPORT) == BST_CHECKED)
                    {
                        g_Config->autoimport = AUTOIMPORT_ASK;
                    }

                    if (IsDlgButtonChecked(hwnd, ID_IMPORTALWAYS) == BST_CHECKED)
                    {
                        g_Config->autoimport = AUTOIMPORT_ALWAYS;
                    }

                    if (IsDlgButtonChecked(hwnd, ID_DONOTHING) == BST_CHECKED)
                    {
                        g_Config->autoimport = AUTOIMPORT_DISABLED;
                    }

                    if (IsDlgButtonChecked(hwnd, ID_USEMASKS) == BST_CHECKED)
                    {
                        g_Config->use_masks = TRUE;
                    }
                    else
                    {
                        g_Config->use_masks = FALSE;
                    }

                    if (IsDlgButtonChecked(hwnd, ID_DEMANGLE) == BST_CHECKED)
                    {
                        g_Config->demangle = TRUE;
                    }
                    else
                    {
                        g_Config->demangle = FALSE;
                    }

                    list_free_masks(g_Config->masks);
                    g_Config->masks = list_create();

                    count = SendMessage(g_hwndMaskList, LB_GETCOUNT, 0, 0);

                    for (counter = 0; counter < count; counter++)
                    {
                        SendMessage(g_hwndMaskList, LB_GETTEXT, counter, (LPARAM)buffer);
                        list_add_mask(g_Config->masks, buffer);
                    }

                    config_create(config_locate(buffer), g_Config);

                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                    break;

                case ID_DELETE:
                    index = SendMessage(g_hwndMaskList, LB_GETCURSEL, 0, 0);
                    SendMessage(g_hwndMaskList, LB_DELETESTRING, index, 0);

                    if (SendMessage(g_hwndMaskList, LB_SETCURSEL, index, 0) == LB_ERR)
                    {
                        SendMessage(g_hwndMaskList, LB_SETCURSEL, index - 1, 0);
                    }
                    break;

                case ID_INCREASE:
                    index = SendMessage(g_hwndMaskList, LB_GETCURSEL, 0, 0);

                    if (index > 0)
                    {
                        SendMessage(g_hwndMaskList, LB_GETTEXT, index, (LPARAM)buffer);
                        SendMessage(g_hwndMaskList, LB_INSERTSTRING, index - 1, (LPARAM)buffer);
                        SendMessage(g_hwndMaskList, LB_DELETESTRING, index + 1, 0);
                        SendMessage(g_hwndMaskList, LB_SETCURSEL, index - 1, 0);
                    }
                    break;

                case ID_MAXINCREASE:
                    index = SendMessage(g_hwndMaskList, LB_GETCURSEL, 0, 0);

                    if (index > 0)
                    {
                        SendMessage(g_hwndMaskList, LB_GETTEXT, index, (LPARAM)buffer);
                        SendMessage(g_hwndMaskList, LB_INSERTSTRING, 0, (LPARAM)buffer);
                        SendMessage(g_hwndMaskList, LB_DELETESTRING, index + 1, 0);
                        SendMessage(g_hwndMaskList, LB_SETCURSEL, 0, 0);
                    }
                    break;

                case ID_DECREASE:
                    index = SendMessage(g_hwndMaskList, LB_GETCURSEL, 0, 0);

                    if (index != LB_ERR && index < SendMessage(g_hwndMaskList, LB_GETCOUNT, 0, 0) - 1)
                    {
                        SendMessage(g_hwndMaskList, LB_GETTEXT, index, (LPARAM)buffer);
                        SendMessage(g_hwndMaskList, LB_INSERTSTRING, index + 2, (LPARAM)buffer);
                        SendMessage(g_hwndMaskList, LB_DELETESTRING, index, 0);
                        SendMessage(g_hwndMaskList, LB_SETCURSEL, index + 1, 0);
                    }
                    break;

                case ID_MAXDECREASE:
                    index = SendMessage(g_hwndMaskList, LB_GETCURSEL, 0, 0);
                    count = SendMessage(g_hwndMaskList, LB_GETCOUNT, 0, 0);

                    if (index != LB_ERR && index < count - 1)
                    {
                        SendMessage(g_hwndMaskList, LB_GETTEXT, index, (LPARAM)buffer);
                        SendMessage(g_hwndMaskList, LB_SETCURSEL, SendMessage(g_hwndMaskList, LB_ADDSTRING, 0, (LPARAM)buffer), 0);
                        SendMessage(g_hwndMaskList, LB_DELETESTRING, index, 0);
                    }
                    break;

                case ID_SAVE:
                    count = SendMessage(g_hwndMaskList, LB_GETCOUNT, 0, 0);

                    if (count > 0)
                    {
                        EnableWindow(hwnd, FALSE);

                        buffer[0] = '\0';

                        if (Browsefilename("Save mask list to:", buffer, ".TXT|*.*", 0x80))
                        {
                            if (file = fopen(buffer, "w"))
                            {
                                for (counter = 0; counter < count; counter++)
                                {
                                    SendMessage(g_hwndMaskList, LB_GETTEXT, counter, (LPARAM)buffer);
                                    strcat(buffer, "\n");
                                    fputs(buffer, file);
                                }

                                fclose(file);
                            }
                            else
                            {
                                MessageBox(hwnd, "Failed to open the file for writing", 0, MB_ICONERROR);
                            }
                        }

                        EnableWindow(hwnd, TRUE);
                        SetFocus(g_hwndMaskList);
                    }
                    break;

                case ID_LOAD:
                    EnableWindow(hwnd, FALSE);

                    buffer[0] = '\0';

                    if (Browsefilename("Load mask list from:", buffer, ".TXT|*.*", 0))
                    {
                        if (file = fopen(buffer, "r"))
                        {
                            while (SendMessage(g_hwndMaskList, LB_DELETESTRING, 0, 0) != LB_ERR) {}

                            while (fgets(buffer, TEXTLEN, file))
                            {
                                strtok(buffer, "\n");

                                if (!mask_compile(&msk_tmp, buffer))
                                {
                                    SendMessage(g_hwndMaskList, LB_ADDSTRING, 0, (LPARAM)buffer);

                                    pcre_free(msk_tmp.regex);

                                    if (msk_tmp.extra)
                                    {
                                        pcre_free(msk_tmp.extra);
                                    }
                                }
                            }
                            SendMessage(g_hwndMaskList, LB_SETCURSEL, 0, 0);

                            fclose(file);
                        }
                        else
                        {
                            MessageBox(hwnd, "Failed to open the file", 0, MB_ICONERROR);
                        }
                    }

                    EnableWindow(hwnd, TRUE);
                    SetFocus(g_hwndMaskList);
                    break;

                case ID_ADD:
                    EnableWindow(hwnd, FALSE);

                    buffer[0] = '\0';
                    result = TRUE;

                    while (result)
                    {
                        result = Gettextxy("Add new mask:",
                                           buffer,
                                           0,
                                           INPUTWND_TYPE,
                                           Plugingetvalue(VAL_WINDOWFONT),
                                           GetSystemMetrics(SM_CXSCREEN) / 2,
                                           GetSystemMetrics(SM_CYSCREEN) / 2);

                        if (result > 0)
                        {
                            if (result = mask_compile(&msk_tmp, buffer))
                            {
                                strcpy(errbuf, buffer);
                                strcat(errbuf, "\n\n");
                                mask_error(&msk_tmp, strrchr(errbuf, '\n') + 1);

                                MessageBox(hwnd, errbuf, "Mask syntax error", MB_ICONERROR);
                            }
                            else
                            {
                                SendMessage(g_hwndMaskList, LB_SETCURSEL, SendMessage(g_hwndMaskList, LB_ADDSTRING, 0, (LPARAM)buffer), 0);

                                pcre_free(msk_tmp.regex);

                                if (msk_tmp.extra)
                                {
                                    pcre_free(msk_tmp.extra);
                                }
                            }
                        }
                        else
                        {
                            break;
                        }
                    }

                    EnableWindow(hwnd, TRUE);
                    SetFocus(g_hwndMaskList);
                    break;

                case ID_INSERT:
                    count = SendMessage(g_hwndMaskList, LB_GETCOUNT, 0, 0);

                    if (count > 0)
                    {
                        EnableWindow(hwnd, FALSE);

                        buffer[0] = '\0';
                        result = TRUE;

                        while (result)
                        {
                            result = Gettextxy("Insert new mask:",
                                               buffer,
                                               0,
                                               INPUTWND_TYPE,
                                               Plugingetvalue(VAL_WINDOWFONT),
                                               GetSystemMetrics(SM_CXSCREEN) / 2,
                                               GetSystemMetrics(SM_CYSCREEN) / 2);

                            if (result > 0)
                            {
                                if (result = mask_compile(&msk_tmp, buffer))
                                {
                                    strcpy(errbuf, buffer);
                                    strcat(errbuf, "\n\n");
                                    mask_error(&msk_tmp, strrchr(errbuf, '\n') + 1);

                                    MessageBox(hwnd, errbuf, "Mask syntax error", MB_ICONERROR);
                                }
                                else
                                {
                                    SendMessage(g_hwndMaskList,
                                                LB_SETCURSEL,
                                                SendMessage(g_hwndMaskList,
                                                            LB_INSERTSTRING,
                                                            SendMessage(g_hwndMaskList, LB_GETCURSEL, 0, 0),
                                                            (LPARAM)buffer),
                                                0);

                                    pcre_free(msk_tmp.regex);

                                    if (msk_tmp.extra)
                                    {
                                        pcre_free(msk_tmp.extra);
                                    }
                                }
                            }
                            else
                            {
                                break;
                            }
                        }

                        EnableWindow(hwnd, TRUE);
                        SetFocus(g_hwndMaskList);
                    }
                    break;

                case ID_EDIT:
                    index = SendMessage(g_hwndMaskList, LB_GETCURSEL, 0, 0);

                    if (index != LB_ERR)
                    {
                        EnableWindow(hwnd, FALSE);

                        SendMessage(g_hwndMaskList, LB_GETTEXT, index, (LPARAM)buffer);
                        result = TRUE;

                        while (result)
                        {
                            result = Gettextxy("Edit mask:",
                                               buffer,
                                               0,
                                               INPUTWND_TYPE,
                                               Plugingetvalue(VAL_WINDOWFONT),
                                               GetSystemMetrics(SM_CXSCREEN) / 2,
                                               GetSystemMetrics(SM_CYSCREEN) / 2);

                            if (result > 0)
                            {
                                if (result = mask_compile(&msk_tmp, buffer))
                                {
                                    strcpy(errbuf, buffer);
                                    strcat(errbuf, "\n\n");
                                    mask_error(&msk_tmp, strrchr(errbuf, '\n') + 1);

                                    MessageBox(hwnd, errbuf, "Mask syntax error", MB_ICONERROR);
                                }
                                else
                                {
                                    SendMessage(g_hwndMaskList, LB_DELETESTRING, index, 0);
                                    SendMessage(g_hwndMaskList,
                                                LB_SETCURSEL,
                                                SendMessage(g_hwndMaskList, LB_INSERTSTRING, (WPARAM)index, (LPARAM)buffer),
                                                0);

                                    pcre_free(msk_tmp.regex);

                                    if (msk_tmp.extra)
                                    {
                                        pcre_free(msk_tmp.extra);
                                    }
                                }
                            }
                            else
                            {
                                break;
                            }
                        }

                        EnableWindow(hwnd, TRUE);
                        SetFocus(g_hwndMaskList);
                    }
                    break;
            }
            break;

        case WM_CREATE:
            EnableWindow(g_hwndOlly, FALSE);

            /* facepalm.jpg */
            GetClientRect(hwnd, &rect);
            height = rect.bottom;
            GetWindowRect(hwnd, &rect);
            height = rect.bottom - rect.top - height + OPTWND_WINDOW_HEIGHT;
            SetWindowPos(hwnd, NULL, 0, 0, rect.right - rect.left, height, SWP_NOMOVE | SWP_NOZORDER);

            wnd = CreateWindowEx(0, "Button", "Import objects:", 0x50020007, 4, 0, 200, 64, hwnd, (HMENU)ID_IMPORT, g_hInstance, NULL);
            SendMessage(wnd, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            wnd = CreateWindowEx(0, "Button", "&Comments", 0x50010003, 12, 16, 112, 20, hwnd, (HMENU)ID_COMMENTS, g_hInstance, NULL);
            SendMessage(wnd, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            wnd = CreateWindowEx(0, "Button", "&Labels", 0x50010003, 12, 36, 112, 20, hwnd, (HMENU)ID_LABELS, g_hInstance, NULL);
            SendMessage(wnd, WM_SETFONT, (WPARAM)g_hFont, TRUE);

            wnd = CreateWindowEx(0, "Button", "Collisions:", 0x50020007, 4, 68, 200, 64, hwnd, (HMENU)ID_COLLISIONS, g_hInstance, NULL);
            SendMessage(wnd, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            wnd = CreateWindowEx(0, "Button", "&Skip if collision", 0x50010009, 12, 86, 116, 16, hwnd, (HMENU)ID_SKIP, g_hInstance, NULL);
            SendMessage(wnd, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            wnd = CreateWindowEx(0, "Button", "&Overwrite", 0x50000009, 12, 106, 116, 16, hwnd, (HMENU)ID_OVERWRITE, g_hInstance, NULL);
            SendMessage(wnd, WM_SETFONT, (WPARAM)g_hFont, TRUE);

            wnd = CreateWindowEx(0, "Button", "Read segments from:", 0x50020007, 4, 136, 200, 64, hwnd, (HMENU)ID_READFROM, g_hInstance, NULL);
            SendMessage(wnd, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            wnd = CreateWindowEx(0, "Button", "&File, memory on fail", 0x50010009, 12, 154, 135, 16, hwnd, (HMENU)ID_FILE, g_hInstance, NULL);
            SendMessage(wnd, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            wnd = CreateWindowEx(0, "Button", "&Memory, assume base on fail", 0x50010009, 12, 174, 184, 16, hwnd, (HMENU)ID_MEMORY, g_hInstance, NULL);
            SendMessage(wnd, WM_SETFONT, (WPARAM)g_hFont, TRUE);

            wnd = CreateWindowEx(0, "Button", "If map file found:", 0x50020007, 4, 204, 200, 80, hwnd, (HMENU)ID_AUTOIMPORT, g_hInstance, NULL);
            SendMessage(wnd, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            wnd = CreateWindowEx(0, "Button", "&Ask to import", 0x50000009, 12, 222, 116, 16, hwnd, (HMENU)ID_ASKTOIMPORT, g_hInstance, NULL);
            SendMessage(wnd, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            wnd = CreateWindowEx(0, "Button", "&Import always", 0x50000009, 12, 242, 116, 16, hwnd, (HMENU)ID_IMPORTALWAYS, g_hInstance, NULL);
            SendMessage(wnd, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            wnd = CreateWindowEx(0, "Button", "Do &nothing", 0x50000009, 12, 262, 116, 16, hwnd, (HMENU)ID_DONOTHING, g_hInstance, NULL);
            SendMessage(wnd, WM_SETFONT, (WPARAM)g_hFont, TRUE);

            wnd = CreateWindowEx(0, "Button", "Filter:", 0x50020007, 209, 0, 208, 284, hwnd, (HMENU)ID_FILTER, g_hInstance, NULL);
            SendMessage(wnd, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            g_hwndMaskList = CreateWindowEx(0x200, "ListBox", "", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL, 217, 16, 192, 230, hwnd, (HMENU)ID_MASKS, g_hInstance, NULL);
            SendMessage(g_hwndMaskList, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            SetWindowLongPtr(g_hwndMaskList, GWLP_USERDATA, SetWindowLongPtr(g_hwndMaskList, GWLP_WNDPROC, (LONG)listbox_msgproc));
            SetFocus(g_hwndMaskList);

            wnd = CreateWindowEx(0, "Button", "&Use masks", 0x50010003, 217, 242, 116, 16, hwnd, (HMENU)ID_USEMASKS, g_hInstance, NULL);
            SendMessage(wnd, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            wnd = CreateWindowEx(0, "Button", "D&emangle names", 0x50010003, 217, 262, 116, 16, hwnd, (HMENU)ID_DEMANGLE, g_hInstance, NULL);
            SendMessage(wnd, WM_SETFONT, (WPARAM)g_hFont, TRUE);

            wnd = CreateWindowEx(0, "Button", "Apply", 0x50012F00, 277, 288, 68, 20, hwnd, (HMENU)ID_APPLY, g_hInstance, NULL);
            SendMessage(wnd, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            wnd = CreateWindowEx(0, "Button", "Cancel", 0x50012F00, 349, 288, 68, 20, hwnd, (HMENU)ID_CANCEL, g_hInstance, NULL);
            SendMessage(wnd, WM_SETFONT, (WPARAM)g_hFont, TRUE);

            if (g_Config->comments)
            {
                CheckDlgButton(hwnd, ID_COMMENTS, BST_CHECKED);
            }
            else
            {
                CheckDlgButton(hwnd, ID_COMMENTS, BST_UNCHECKED);
            }

            if (g_Config->labels)
            {
                CheckDlgButton(hwnd, ID_LABELS, BST_CHECKED);
            }
            else
            {
                CheckDlgButton(hwnd, ID_LABELS, BST_UNCHECKED);
            }

            if (g_Config->check_collisions)
            {
                CheckDlgButton(hwnd, ID_SKIP, BST_CHECKED);
                CheckDlgButton(hwnd, ID_OVERWRITE, BST_UNCHECKED);
            }
            else
            {
                CheckDlgButton(hwnd, ID_SKIP, BST_UNCHECKED);
                CheckDlgButton(hwnd, ID_OVERWRITE, BST_CHECKED);
            }

            if (g_Config->read_file_segments)
            {
                CheckDlgButton(hwnd, ID_FILE, BST_CHECKED);
                CheckDlgButton(hwnd, ID_MEMORY, BST_UNCHECKED);
            }
            else
            {
                CheckDlgButton(hwnd, ID_FILE, BST_UNCHECKED);
                CheckDlgButton(hwnd, ID_MEMORY, BST_CHECKED);
            }

            if (g_Config->autoimport == AUTOIMPORT_ASK)
            {
                CheckDlgButton(hwnd, ID_ASKTOIMPORT, BST_CHECKED);
                CheckDlgButton(hwnd, ID_IMPORTALWAYS, BST_UNCHECKED);
                CheckDlgButton(hwnd, ID_DONOTHING, BST_UNCHECKED);
            }
            else if (g_Config->autoimport == AUTOIMPORT_ALWAYS)
            {
                CheckDlgButton(hwnd, ID_IMPORTALWAYS, BST_CHECKED);
                CheckDlgButton(hwnd, ID_ASKTOIMPORT, BST_UNCHECKED);
                CheckDlgButton(hwnd, ID_DONOTHING, BST_UNCHECKED);
            }
            else
            {
                CheckDlgButton(hwnd, ID_DONOTHING, BST_CHECKED);
                CheckDlgButton(hwnd, ID_ASKTOIMPORT, BST_UNCHECKED);
                CheckDlgButton(hwnd, ID_IMPORTALWAYS, BST_UNCHECKED);
            }

            if (g_Config->use_masks)
            {
                CheckDlgButton(hwnd, ID_USEMASKS, BST_CHECKED);
            }
            else
            {
                CheckDlgButton(hwnd, ID_USEMASKS, BST_UNCHECKED);
            }

            if (g_Config->demangle)
            {
                CheckDlgButton(hwnd, ID_DEMANGLE, BST_CHECKED);
            }
            else
            {
                CheckDlgButton(hwnd, ID_DEMANGLE, BST_UNCHECKED);
            }

            msk = (pmask_t)g_Config->masks->first;

            while (msk)
            {
                SendMessage(g_hwndMaskList, LB_ADDSTRING, 0, (LPARAM)msk->buffer);

                msk = msk->next;
            }

            SendMessage(g_hwndMaskList, LB_SETCURSEL, 0, 0);
            break;

        case WM_CLOSE:
            EnableWindow(g_hwndOlly, TRUE);
            SetFocus(g_hwndOlly);
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wparam, lparam);
    }
    return FALSE;
}

void configwnd_create()
{
    WNDCLASS wc;
    HWND     wnd;
    MSG      msg;
    HACCEL   acctable = CreateAcceleratorTable((LPACCEL)&c_AccelTable, sizeof(c_AccelTable) / sizeof(ACCEL));

    InitCommonControls();

    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon         = LoadIcon(g_hInstance, OPTWND_ICON_NAME);
    wc.hInstance     = g_hInstance;
    wc.lpszMenuName  = NULL;
    wc.style         = CS_PARENTDC | CS_DBLCLKS;
    wc.lpfnWndProc   = configwnd_msgproc;
    wc.lpszClassName = OPTWND_CLASS_NAME;

    RegisterClass(&wc);

    g_hFont = CreateFont(-10,
                         0,
                         0,
                         0,
                         FW_NORMAL,
                         0,
                         0,
                         0,
                         ANSI_CHARSET,
                         OUT_DEFAULT_PRECIS,
                         CLIP_DEFAULT_PRECIS,
                         DEFAULT_QUALITY,
                         DEFAULT_PITCH | FF_DONTCARE,
                         OPTWND_FONT_NAME);

    wnd = CreateWindowEx(0,
                         OPTWND_CLASS_NAME,
                         OPTWND_WINDOW_NAME,
                         WS_VISIBLE | WS_SYSMENU | WS_OVERLAPPED | DS_SYSMODAL,
                         (GetSystemMetrics(SM_CXSCREEN) - OPTWND_WINDOW_WIDTH) / 2,
                         (GetSystemMetrics(SM_CYSCREEN) - OPTWND_WINDOW_HEIGHT) / 2,
                         OPTWND_WINDOW_WIDTH,
                         OPTWND_WINDOW_HEIGHT,
                         g_hwndOlly,
                         NULL,
                         g_hInstance,
                         NULL);

    ShowWindow(wnd, SW_SHOWNORMAL);
    UpdateWindow(wnd);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(wnd, acctable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    DeleteObject(g_hFont);
    UnregisterClass(OPTWND_CLASS_NAME, g_hInstance);
    DestroyAcceleratorTable(acctable);
}

int module_read_file_seg(pmodule_t module)
{
    HANDLE                hFile,
                          hMapping;
    LPVOID                pMapping;
    PIMAGE_DOS_HEADER     dos;
    PIMAGE_NT_HEADERS     nt;
    PIMAGE_SECTION_HEADER sh;
    ULONG                 rs = MODULE_SUCCESS,
                          cntr_seg;

    hFile = CreateFile(module->name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        if (hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, 0))
        {
            if (pMapping = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0))
            {
                dos = (PIMAGE_DOS_HEADER)pMapping;
                nt = (PIMAGE_NT_HEADERS)((ULONG)dos + dos->e_lfanew);

                module->nseg = nt->FileHeader.NumberOfSections + 1;
                module->segments = (PULONG)malloc(module->nseg * sizeof(ULONG));
                module->segments[0] = 0;

                sh = IMAGE_FIRST_SECTION(nt);

                for (cntr_seg = 1; cntr_seg < module->nseg; cntr_seg++)
                {
                    module->segments[cntr_seg] = sh->VirtualAddress;
                    sh++;
                }

                UnmapViewOfFile(pMapping);
                CloseHandle(hMapping);
                CloseHandle(hFile);
            }
            else
            {
                CloseHandle(hMapping);
                CloseHandle(hFile);

                rs = MODULE_FILE_MAPPING_FAILURE;
            }
        }
        else
        {
            CloseHandle(hFile);

            rs = MODULE_FILE_MAPPING_FAILURE;
        }
    }
    else
    {   
        rs = MODULE_FILE_SHARING_VIOLATION;        
    }

    return rs;
}

#define READMEM(buf, addr, size, onfailure) if (Readmemory(buf, (ulong)addr, size, MM_RESILENT) < size){ onfailure; return MODULE_MEMORY_READ_FAILURE;}

int module_read_mem_seg(pmodule_t module)
{
    IMAGE_DOS_HEADER     dos;
    IMAGE_NT_HEADERS     nt;
    IMAGE_SECTION_HEADER sh,
                        *psh;
    ULONG                cntr_seg;

    READMEM(&dos, module->base, sizeof(IMAGE_DOS_HEADER));
    READMEM(&nt, module->base + dos.e_lfanew, sizeof(IMAGE_NT_HEADERS));

    module->nseg = nt.FileHeader.NumberOfSections + 1;
    module->segments = (PULONG)malloc(module->nseg * sizeof(ULONG));
    module->segments[0] = 0;

    psh = (PIMAGE_SECTION_HEADER)(module->base + dos.e_lfanew + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader) + nt.FileHeader.SizeOfOptionalHeader);

    for (cntr_seg = 1; cntr_seg < module->nseg; cntr_seg++)
    {
        READMEM(&sh, psh, sizeof(IMAGE_SECTION_HEADER), free(module->segments));

        if (sh.VirtualAddress > module->size)
        {
            module->segments[cntr_seg] = 0;
        }
        else
        {
            module->segments[cntr_seg] = sh.VirtualAddress;
        }

        psh++;
    }

    return MODULE_SUCCESS;
}

ULONG module_manual_base(ULONG initial)
{
    ULONG rs = initial;

    while (!Getlongxy("Specify region base",
                      (ulong*)&rs,
                      sizeof(ULONG),
                      0,
                      DIA_HEXONLY,
                      (GetSystemMetrics(SM_CXSCREEN) - MODULE_MANUAL_WIDTH) / 2,
                      (GetSystemMetrics(SM_CYSCREEN) - MODULE_MANUAL_HEIGHT) / 2))
    {
        if (rs) return rs;

        MessageBox(g_hwndOlly, "Incorrect value", NULL, MB_ICONERROR);
    }

    return 0;
}

pmodule_t module_init(int* err)
{   
    ULONG     cbase,
              csize;
    t_module* dbg_mod;
    pmodule_t module = (pmodule_t)malloc(sizeof(module_t));

    Getdisassemblerrange(&cbase, &csize);

    if (cbase)
    {
        if (dbg_mod = Findmodule(cbase))
        {
            module->base = (ULONG)dbg_mod->base;
            module->size = (ULONG)dbg_mod->size;
            module->name = (TCHAR*)&dbg_mod->path;
        }
        else if (module->base = module_manual_base(cbase))
        {
            /* Name applying function does not check whether the calculated virtual address
               belongs to the applied module or not, so we just put max size here */
            module->size = MODULE_SIZE_MAX;
            module->name = (TCHAR*)MODULE_UNKNOWN_NAME;
        }
        else
        {
            *err = MODULE_BASE_NOT_FOUND;
            free(module);

            return NULL;
        }

        if (g_Config->read_file_segments)
        {
            if (*err = module_read_file_seg(module))
            {
                *err = module_read_mem_seg(module);
            }
        }
        else
        {
            if (*err = module_read_mem_seg(module))
            {
                module->nseg = 4;
                module->segments = (PULONG)malloc(4 * sizeof(ULONG));
                module->segments[0] = 0;
                module->segments[1] = 0;
                module->segments[2] = 0;
                module->segments[3] = 0;

                *err = MODULE_SUCCESS;
            }
        }
    }
    else
    {
        *err = MODULE_OUT_OF_RANGE;
        free(module);

        module = NULL;
    }

    return module;
}

void module_error(int err)
{
    switch (err)
    {
        case MODULE_OUT_OF_RANGE:
            Flash("You are not viewing any module");
            break;

        case MODULE_BASE_NOT_FOUND:
            Flash("Failed to obtain module base");
            Addtolist(0, 1, "Failed to obtain module base");
            break;

        case MODULE_FILE_MAPPING_FAILURE:
            Flash("Failed to create file mapping");
            Addtolist(0, 1, "Failed to create file mapping");
            break;

        case MODULE_FILE_SHARING_VIOLATION:
            Flash("Failed to obtain file handle");
            Addtolist(0, 1, "Failed to obtain file handle");
            break;

        case MODULE_MEMORY_READ_FAILURE:
            Flash("Failed to read segments information from memory");
            Addtolist(0, 1, "Failed to read segments information from memory");
            break;

        default:
            break;
    }
}

void module_free(pmodule_t module)
{
    free(module->segments);
    free(module);
}

void mapfile_apply(pmodule_t module, plist_t names)
{
    TCHAR   undecorated[MAPBUFLEN];
    ULONG   total = 0,
            filtered = 0,
            applied = 0,
            addr;
    int     result;
    pname_t nm,
            nm_last;
    plist_t rmtable;
    
    Addtolist(0, 0, "Applying names from map file to module '%s'", module->name);

    if (!g_Config->check_collisions)
    {
        rmtable = list_create();
    }

    for (nm = (pname_t)names->first; nm; nm = nm->next, total++)
    {
        if (nm->segment < module->nseg)
        {
            if (g_Config->demangle && (result = Demanglename(nm->buffer, NM_LIBRARY, undecorated)))
            {
                free(nm->buffer);
                nm->size = (result + 1) * sizeof(TCHAR);
                nm->buffer = (TCHAR*)malloc(nm->size);
                strcpy(nm->buffer, undecorated);
            }

            addr = module->base + module->segments[nm->segment] + nm->offset;

            if (g_Config->use_masks)
            {
                if (result = mask_filter(nm))
                {
                    filtered++;

                    if (result == FILTER_SKIP)
                    {
                        if (!g_Config->check_collisions &&
                            /* Findname for NM_ANYNAME fails everytime, dunno why */
                            (Findname(addr, NM_COMMENT, NULL) || Findname(addr, NM_LABEL, NULL)))
                        {
                            list_add_name(rmtable, NULL, 0, nm->segment, nm->offset);
                        }

                        continue;
                    }
                }
            }

            if (g_Config->comments)
            {
                if (g_Config->check_collisions)
                {
                    if (!Findname(addr, NM_COMMENT, NULL))
                    {
                        /* Quickinsertname returns zero if the name had been inserted and -1 on error */
                        applied += Quickinsertname(addr, NM_COMMENT, nm->buffer) + 1;
                    }
                }
                else
                {
                    applied += Quickinsertname(addr, NM_COMMENT, nm->buffer) + 1;
                }
            }

            if (g_Config->labels)
            {
                if (g_Config->check_collisions)
                {
                    /* Do not increase "applied" variable again if it has been done in comment insertion block above */
                    if (!Findlabel(addr, NULL) && !Quickinsertname(addr, NM_LABEL, nm->buffer) && !g_Config->comments)
                    {
                        applied++;
                    }
                }
                else if (!Quickinsertname(addr, NM_LABEL, nm->buffer) && !g_Config->comments)
                {
                    applied++;
                }
            }
        }

        Progress(total * 1000 / names->count, "Inserting names");
    }

    Progress(0, "Merging names");
    Mergequicknames();

    if (!g_Config->check_collisions)
    {
        Infoline("Cleaning skipped records");

        nm = (pname_t)rmtable->first;

        while (nm)
        {
            addr = module->base + module->segments[nm->segment] + nm->offset;

            if (g_Config->comments)
            {
                Insertname(addr, NM_COMMENT, "");
            }
            if (g_Config->labels)
            {
                Insertname(addr, NM_LABEL, "");
            }

            nm_last = nm;
            nm = nm->next;
            /* Manual list_free_names expansion to speed it up somehow */
            free(nm_last);
        }
    }

    Infoline("Total loaded: %d, Names applied: %d, Names filtered: %d", total, applied, filtered);
    Addtolist(0, -1, "  Total loaded: %d, Names applied: %d, Names filtered: %d", total, applied, filtered);        
}

#endif

plist_t mapfile_parse(TCHAR* path)
{
    FILE*   file = fopen(path, "r");
    plist_t list = NULL;
    BYTE    segment;
    ULONG   offset,
            len,
            counter,
            ntokens = 0;
    TCHAR*  buffer,
           *oldbuffer;

    if (file)
    {
        oldbuffer = buffer = malloc(MAPBUFLEN * sizeof(TCHAR));

        for (counter = 0; counter < 3; counter++)
        {
            while (fgets(buffer, MAPBUFLEN, file))
            {
                buffer[strlen(buffer) - 1] = '\0';

                if (strstr(buffer, "Address"))
                {
                    if (!list)
                    {
                        list = list_create();
                    }

                    ntokens = 1;

                    strtok(buffer, " ");

                    while (strtok(NULL, " "))
                    {
                        ntokens++;
                    }

                    fgets(buffer, MAPBUFLEN, file);
                    break;
                }
                else if (strstr(buffer, "Static symbols"))
                {
                    fgets(buffer, MAPBUFLEN, file);
                    break;
                }
                else
                {
                    continue;
                }
            }

            if (ntokens)
            {
                while (fgets(buffer, MAPBUFLEN, file))
                {
                    if (buffer[0] == ' ')
                    {
                        buffer++;
                        buffer[4] = '\0';
                        segment = (BYTE)strtol(buffer, NULL, 16);
                        buffer += 5;
                        buffer[8] = '\0';
                        offset = (ULONG)strtol(buffer, NULL, 16);
                        buffer += 9;

                        while (*buffer == ' ')
                        {
                            buffer++;
                        }
                        /*
                            ntokens contains a number of words in the column names line
                            in the name table. If there are 4 words: "Address", "Publics",
                            "by" and "Value" (or "Name") there are only 2 columns in the
                            table. If there are more, then every name should be delimited
                            from another column by space character and we have to handle that
                        */
                        if (ntokens > 4)
                        {
                            for (len = 0; buffer[len] != '\0'; len++)
                            {
                                if (buffer[len] == ' ')
                                {
                                    buffer[len] = '\0';
                                    break;
                                }
                            }
                        }
                        else
                        {
                            len = strlen(buffer) - 1;
                            buffer[len] = '\0';
                        }

                        list_add_name(list, buffer, len, segment, offset);
                        buffer = oldbuffer;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
        free(buffer);

        fclose(file);
    }
    return list;
}

#ifndef MASKTEST

BOOL mapfile_browse(TCHAR* path)
{
    OPENFILENAME ofn;
    TCHAR        fpath[MAX_PATH];
    BOOL         rs = FALSE;

    path[0] = '\0';

    memset(&ofn, 0, sizeof(OPENFILENAME));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = (HWND)Plugingetvalue(VAL_HWMAIN);
    ofn.lpstrFile = path;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = "Map Files\0*.MAP\0All Files\0*.*\0\0";
    ofn.lpstrDefExt = "MAP";
    ofn.lpstrTitle = "Select map file to import";

    strcpy(fpath, (TCHAR*)Plugingetvalue(VAL_EXEFILENAME));
    *(strrchr(fpath, '\\') + 1) = '\0';

    ofn.lpstrInitialDir = fpath;
    ofn.Flags = OFN_HIDEREADONLY;

    while (rs = GetOpenFileName(&ofn))
    {
        if (GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES)
        {
#ifdef IMMDBG
            Flash("Could not locate file \"%s\"", path);
#else
            Error("Could not locate file \"%s\"", path);
#endif
        }
        else
        {
            break;
        }
    }

    return rs;
}

BOOL WINAPI DllMain(HINSTANCE hi, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hi);
        g_hInstDLL = hi;
    }

    return TRUE;
}

int _export cdecl ODBG_Plugininit(int ollydbgversion, HWND hw, ULONG *features)
{
    TCHAR path[MAX_PATH];

    g_Config = config_parse(config_locate(path));
    g_hInstance = GetModuleHandle(NULL);
    g_hwndOlly = hw;
    g_SessionStarted = FALSE;
    g_Autoloaded = FALSE;
    Addtolist(0, 0, "mapimp plugin v1.0");
    Addtolist(0, -1, "  tPORt, 2009-2012");

    return 0;
}

int _export cdecl ODBG_Plugindata(char shortname[32])
{
    strcpy(shortname, "mapimp");

    return PLUGIN_VERSION;
}

int _export cdecl ODBG_Pluginmenu(int origin, char data[4096], void *item)
{
    if (origin == PM_MAIN)
    {
        strcpy(data, "0 &Import map, 1 &Options |3 &About");

        return 1;
    }

    return 0;
}

void _export cdecl ODBG_Pluginreset()
{
    g_SessionStarted = FALSE;
}

int _export cdecl ODBG_Pausedex(int reason, int extdata, t_reg *reg, DEBUG_EVENT *debugevent)
{
    plist_t   names;
    pmodule_t module;
    TCHAR     path[MAX_PATH];
    TCHAR*    pos;
    int       err;

    if ((reason & PP_INT3BREAK) && !g_SessionStarted && !g_Autoloaded)
    {
        g_SessionStarted = TRUE;

        strcpy(path, (TCHAR*)Plugingetvalue(VAL_EXEFILENAME));

        if (pos = strrchr(path, '.'))
        {
            strcpy(pos, ".map");
        }
        else
        {
            strcat(path, ".map");
        }

        if (GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES)
        {
            g_Autoloaded = TRUE;

            if ((g_Config->autoimport == AUTOIMPORT_ASK) &&
                (MessageBox(g_hwndOlly,
                            "Corresponding map file found. Do you want to import it now?",
                            "mapimp",
                            MB_YESNO | MB_ICONQUESTION) == IDYES) || (g_Config->autoimport == AUTOIMPORT_ALWAYS))
            {
                module = module_init(&err);

                if (!err)
                {
                    if (names = mapfile_parse(path))
                    {
                        mapfile_apply(module, names);
                        list_free_names(names);

                        Setcpu(0, 0, 0, 0, CPU_ASMFOCUS);
                    }
                    else
                    {
                        Flash("Failed to open the file");
                    }
                }
                else
                {
                    module_error(err);
                }
            }
        }
    }

    return 0;
}

void _export cdecl ODBG_Pluginaction(int origin, int action, void *item)
{
    plist_t   names;
    pmodule_t module;
    TCHAR     path[TEXTLEN];
    t_status  status;
    int       err;

    if (origin == PM_MAIN)
    {
        switch (action)
        {
            case ACTION_IMPORT:
                status = Getstatus();

                if (status && status != STAT_FINISHED && status != STAT_CLOSING)
                {
                    module = module_init(&err);

                    if (!err)
                    {
                        if (mapfile_browse(path))
                        {
                            if (names = mapfile_parse(path))
                            {
                                mapfile_apply(module, names);
                                list_free_names(names);

                                Setcpu(0, 0, 0, 0, CPU_ASMFOCUS);
                            }
                            else
                            {
                                Flash("Failed to open the file");
                            }
                        }
                    }
                    else
                    {
                        module_error(err);
                    }
                }
                else
                {
                    Flash("Start the debugging session first");
                }
                break;

            case ACTION_OPTIONS:
                configwnd_create();
                break;

            case ACTION_ABOUT:
                MessageBox(g_hwndOlly, c_About, "About mapimp", MB_ICONINFORMATION);
                break;

            default:
                break;
        }
    }
}

int _export cdecl ODBG_Pluginshortcut(int origin, int ctrl, int alt, int shift, int key, void* item)
{   
    if (origin == PM_MAIN && ctrl && key == 'I')
    {
        if (shift)
        {
            ODBG_Pluginaction(origin, ACTION_IMPORT, NULL);

            return TRUE;
        }
        else
        {
            ODBG_Pluginaction(origin, ACTION_OPTIONS, NULL);

            return TRUE;
        }
    }

    return FALSE;
}

void _export cdecl ODBG_Pluginsaveudd(t_module *pmod, int ismainmodule)
{
    if (ismainmodule)
    {
        Pluginsaverecord(TAG_MAPIMP, sizeof(BOOL), &g_Autoloaded);
    }
}

int _export cdecl ODBG_Pluginuddrecord(t_module *pmod, int ismainmodule, ulong tag, ulong size, void *data)
{
    if (ismainmodule && tag == TAG_MAPIMP)
    {
        g_Autoloaded = *(LPBOOL)data;

        return 1;
    }

    return 0;
}

void _export cdecl ODBG_Plugindestroy()
{
    list_free_masks(g_Config->masks);
    free(g_Config);
}

#endif