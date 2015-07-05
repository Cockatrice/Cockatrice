#include <QSettings>
#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QDebug>

#include "dlg_edit_user.h"

DlgEditUser::DlgEditUser(QWidget *parent, QString email, int gender, QString country, QString realName)
    : QDialog(parent)
{
    emailLabel = new QLabel(tr("Email:"));
    emailEdit = new QLineEdit();
    emailLabel->setBuddy(emailEdit);
    emailEdit->setText(email);

    genderLabel = new QLabel(tr("Gender:"));
    genderEdit = new QComboBox();
    genderLabel->setBuddy(genderEdit);
    genderEdit->insertItem(0, QIcon(":/resources/genders/unknown.svg"), tr("Undefined"));
    genderEdit->insertItem(1, QIcon(":/resources/genders/male.svg"), tr("Male"));
    genderEdit->insertItem(2, QIcon(":/resources/genders/female.svg"), tr("Female"));
    genderEdit->setCurrentIndex(gender + 1);

    countryLabel = new QLabel(tr("Country:"));
    countryEdit = new QComboBox();
    countryLabel->setBuddy(countryEdit);
    countryEdit->insertItem(0, tr("Undefined"));
    countryEdit->addItem(QPixmap(":/resources/countries/ad.svg"), "ad");
    countryEdit->addItem(QIcon(":/resources/countries/ae.svg"), "ae");
    countryEdit->addItem(QIcon(":/resources/countries/af.svg"), "af");
    countryEdit->addItem(QIcon(":/resources/countries/ag.svg"), "ag");
    countryEdit->addItem(QIcon(":/resources/countries/ai.svg"), "ai");
    countryEdit->addItem(QIcon(":/resources/countries/al.svg"), "al");
    countryEdit->addItem(QIcon(":/resources/countries/am.svg"), "am");
    countryEdit->addItem(QIcon(":/resources/countries/ao.svg"), "ao");
    countryEdit->addItem(QIcon(":/resources/countries/aq.svg"), "aq");
    countryEdit->addItem(QIcon(":/resources/countries/ar.svg"), "ar");
    countryEdit->addItem(QIcon(":/resources/countries/as.svg"), "as");
    countryEdit->addItem(QIcon(":/resources/countries/at.svg"), "at");
    countryEdit->addItem(QIcon(":/resources/countries/au.svg"), "au");
    countryEdit->addItem(QIcon(":/resources/countries/aw.svg"), "aw");
    countryEdit->addItem(QIcon(":/resources/countries/ax.svg"), "ax");
    countryEdit->addItem(QIcon(":/resources/countries/az.svg"), "az");
    countryEdit->addItem(QIcon(":/resources/countries/ba.svg"), "ba");
    countryEdit->addItem(QIcon(":/resources/countries/bb.svg"), "bb");
    countryEdit->addItem(QIcon(":/resources/countries/bd.svg"), "bd");
    countryEdit->addItem(QIcon(":/resources/countries/be.svg"), "be");
    countryEdit->addItem(QIcon(":/resources/countries/bf.svg"), "bf");
    countryEdit->addItem(QIcon(":/resources/countries/bg.svg"), "bg");
    countryEdit->addItem(QIcon(":/resources/countries/bh.svg"), "bh");
    countryEdit->addItem(QIcon(":/resources/countries/bi.svg"), "bi");
    countryEdit->addItem(QIcon(":/resources/countries/bj.svg"), "bj");
    countryEdit->addItem(QIcon(":/resources/countries/bl.svg"), "bl");
    countryEdit->addItem(QIcon(":/resources/countries/bm.svg"), "bm");
    countryEdit->addItem(QIcon(":/resources/countries/bn.svg"), "bn");
    countryEdit->addItem(QIcon(":/resources/countries/bo.svg"), "bo");
    countryEdit->addItem(QIcon(":/resources/countries/bq.svg"), "bq");
    countryEdit->addItem(QIcon(":/resources/countries/br.svg"), "br");
    countryEdit->addItem(QIcon(":/resources/countries/bs.svg"), "bs");
    countryEdit->addItem(QIcon(":/resources/countries/bt.svg"), "bt");
    countryEdit->addItem(QIcon(":/resources/countries/bv.svg"), "bv");
    countryEdit->addItem(QIcon(":/resources/countries/bw.svg"), "bw");
    countryEdit->addItem(QIcon(":/resources/countries/by.svg"), "by");
    countryEdit->addItem(QIcon(":/resources/countries/bz.svg"), "bz");
    countryEdit->addItem(QIcon(":/resources/countries/ca.svg"), "ca");
    countryEdit->addItem(QIcon(":/resources/countries/cc.svg"), "cc");
    countryEdit->addItem(QIcon(":/resources/countries/cd.svg"), "cd");
    countryEdit->addItem(QIcon(":/resources/countries/cf.svg"), "cf");
    countryEdit->addItem(QIcon(":/resources/countries/cg.svg"), "cg");
    countryEdit->addItem(QIcon(":/resources/countries/ch.svg"), "ch");
    countryEdit->addItem(QIcon(":/resources/countries/ci.svg"), "ci");
    countryEdit->addItem(QIcon(":/resources/countries/ck.svg"), "ck");
    countryEdit->addItem(QIcon(":/resources/countries/cl.svg"), "cl");
    countryEdit->addItem(QIcon(":/resources/countries/cm.svg"), "cm");
    countryEdit->addItem(QIcon(":/resources/countries/cn.svg"), "cn");
    countryEdit->addItem(QIcon(":/resources/countries/co.svg"), "co");
    countryEdit->addItem(QIcon(":/resources/countries/cr.svg"), "cr");
    countryEdit->addItem(QIcon(":/resources/countries/cu.svg"), "cu");
    countryEdit->addItem(QIcon(":/resources/countries/cv.svg"), "cv");
    countryEdit->addItem(QIcon(":/resources/countries/cw.svg"), "cw");
    countryEdit->addItem(QIcon(":/resources/countries/cx.svg"), "cx");
    countryEdit->addItem(QIcon(":/resources/countries/cy.svg"), "cy");
    countryEdit->addItem(QIcon(":/resources/countries/cz.svg"), "cz");
    countryEdit->addItem(QIcon(":/resources/countries/de.svg"), "de");
    countryEdit->addItem(QIcon(":/resources/countries/dj.svg"), "dj");
    countryEdit->addItem(QIcon(":/resources/countries/dk.svg"), "dk");
    countryEdit->addItem(QIcon(":/resources/countries/dm.svg"), "dm");
    countryEdit->addItem(QIcon(":/resources/countries/do.svg"), "do");
    countryEdit->addItem(QIcon(":/resources/countries/dz.svg"), "dz");
    countryEdit->addItem(QIcon(":/resources/countries/ec.svg"), "ec");
    countryEdit->addItem(QIcon(":/resources/countries/ee.svg"), "ee");
    countryEdit->addItem(QIcon(":/resources/countries/eg.svg"), "eg");
    countryEdit->addItem(QIcon(":/resources/countries/eh.svg"), "eh");
    countryEdit->addItem(QIcon(":/resources/countries/er.svg"), "er");
    countryEdit->addItem(QIcon(":/resources/countries/es.svg"), "es");
    countryEdit->addItem(QIcon(":/resources/countries/et.svg"), "et");
    countryEdit->addItem(QIcon(":/resources/countries/fi.svg"), "fi");
    countryEdit->addItem(QIcon(":/resources/countries/fj.svg"), "fj");
    countryEdit->addItem(QIcon(":/resources/countries/fk.svg"), "fk");
    countryEdit->addItem(QIcon(":/resources/countries/fm.svg"), "fm");
    countryEdit->addItem(QIcon(":/resources/countries/fo.svg"), "fo");
    countryEdit->addItem(QIcon(":/resources/countries/fr.svg"), "fr");
    countryEdit->addItem(QIcon(":/resources/countries/ga.svg"), "ga");
    countryEdit->addItem(QIcon(":/resources/countries/gb.svg"), "gb");
    countryEdit->addItem(QIcon(":/resources/countries/gd.svg"), "gd");
    countryEdit->addItem(QIcon(":/resources/countries/ge.svg"), "ge");
    countryEdit->addItem(QIcon(":/resources/countries/gf.svg"), "gf");
    countryEdit->addItem(QIcon(":/resources/countries/gg.svg"), "gg");
    countryEdit->addItem(QIcon(":/resources/countries/gh.svg"), "gh");
    countryEdit->addItem(QIcon(":/resources/countries/gi.svg"), "gi");
    countryEdit->addItem(QIcon(":/resources/countries/gl.svg"), "gl");
    countryEdit->addItem(QIcon(":/resources/countries/gm.svg"), "gm");
    countryEdit->addItem(QIcon(":/resources/countries/gn.svg"), "gn");
    countryEdit->addItem(QIcon(":/resources/countries/gp.svg"), "gp");
    countryEdit->addItem(QIcon(":/resources/countries/gq.svg"), "gq");
    countryEdit->addItem(QIcon(":/resources/countries/gr.svg"), "gr");
    countryEdit->addItem(QIcon(":/resources/countries/gs.svg"), "gs");
    countryEdit->addItem(QIcon(":/resources/countries/gt.svg"), "gt");
    countryEdit->addItem(QIcon(":/resources/countries/gu.svg"), "gu");
    countryEdit->addItem(QIcon(":/resources/countries/gw.svg"), "gw");
    countryEdit->addItem(QIcon(":/resources/countries/gy.svg"), "gy");
    countryEdit->addItem(QIcon(":/resources/countries/hk.svg"), "hk");
    countryEdit->addItem(QIcon(":/resources/countries/hm.svg"), "hm");
    countryEdit->addItem(QIcon(":/resources/countries/hn.svg"), "hn");
    countryEdit->addItem(QIcon(":/resources/countries/hr.svg"), "hr");
    countryEdit->addItem(QIcon(":/resources/countries/ht.svg"), "ht");
    countryEdit->addItem(QIcon(":/resources/countries/hu.svg"), "hu");
    countryEdit->addItem(QIcon(":/resources/countries/id.svg"), "id");
    countryEdit->addItem(QIcon(":/resources/countries/ie.svg"), "ie");
    countryEdit->addItem(QIcon(":/resources/countries/il.svg"), "il");
    countryEdit->addItem(QIcon(":/resources/countries/im.svg"), "im");
    countryEdit->addItem(QIcon(":/resources/countries/in.svg"), "in");
    countryEdit->addItem(QIcon(":/resources/countries/io.svg"), "io");
    countryEdit->addItem(QIcon(":/resources/countries/iq.svg"), "iq");
    countryEdit->addItem(QIcon(":/resources/countries/ir.svg"), "ir");
    countryEdit->addItem(QIcon(":/resources/countries/is.svg"), "is");
    countryEdit->addItem(QIcon(":/resources/countries/it.svg"), "it");
    countryEdit->addItem(QIcon(":/resources/countries/je.svg"), "je");
    countryEdit->addItem(QIcon(":/resources/countries/jm.svg"), "jm");
    countryEdit->addItem(QIcon(":/resources/countries/jo.svg"), "jo");
    countryEdit->addItem(QIcon(":/resources/countries/jp.svg"), "jp");
    countryEdit->addItem(QIcon(":/resources/countries/ke.svg"), "ke");
    countryEdit->addItem(QIcon(":/resources/countries/kg.svg"), "kg");
    countryEdit->addItem(QIcon(":/resources/countries/kh.svg"), "kh");
    countryEdit->addItem(QIcon(":/resources/countries/ki.svg"), "ki");
    countryEdit->addItem(QIcon(":/resources/countries/km.svg"), "km");
    countryEdit->addItem(QIcon(":/resources/countries/kn.svg"), "kn");
    countryEdit->addItem(QIcon(":/resources/countries/kp.svg"), "kp");
    countryEdit->addItem(QIcon(":/resources/countries/kr.svg"), "kr");
    countryEdit->addItem(QIcon(":/resources/countries/kw.svg"), "kw");
    countryEdit->addItem(QIcon(":/resources/countries/ky.svg"), "ky");
    countryEdit->addItem(QIcon(":/resources/countries/kz.svg"), "kz");
    countryEdit->addItem(QIcon(":/resources/countries/la.svg"), "la");
    countryEdit->addItem(QIcon(":/resources/countries/lb.svg"), "lb");
    countryEdit->addItem(QIcon(":/resources/countries/lc.svg"), "lc");
    countryEdit->addItem(QIcon(":/resources/countries/li.svg"), "li");
    countryEdit->addItem(QIcon(":/resources/countries/lk.svg"), "lk");
    countryEdit->addItem(QIcon(":/resources/countries/lr.svg"), "lr");
    countryEdit->addItem(QIcon(":/resources/countries/ls.svg"), "ls");
    countryEdit->addItem(QIcon(":/resources/countries/lt.svg"), "lt");
    countryEdit->addItem(QIcon(":/resources/countries/lu.svg"), "lu");
    countryEdit->addItem(QIcon(":/resources/countries/lv.svg"), "lv");
    countryEdit->addItem(QIcon(":/resources/countries/ly.svg"), "ly");
    countryEdit->addItem(QIcon(":/resources/countries/ma.svg"), "ma");
    countryEdit->addItem(QIcon(":/resources/countries/mc.svg"), "mc");
    countryEdit->addItem(QIcon(":/resources/countries/md.svg"), "md");
    countryEdit->addItem(QIcon(":/resources/countries/me.svg"), "me");
    countryEdit->addItem(QIcon(":/resources/countries/mf.svg"), "mf");
    countryEdit->addItem(QIcon(":/resources/countries/mg.svg"), "mg");
    countryEdit->addItem(QIcon(":/resources/countries/mh.svg"), "mh");
    countryEdit->addItem(QIcon(":/resources/countries/mk.svg"), "mk");
    countryEdit->addItem(QIcon(":/resources/countries/ml.svg"), "ml");
    countryEdit->addItem(QIcon(":/resources/countries/mm.svg"), "mm");
    countryEdit->addItem(QIcon(":/resources/countries/mn.svg"), "mn");
    countryEdit->addItem(QIcon(":/resources/countries/mo.svg"), "mo");
    countryEdit->addItem(QIcon(":/resources/countries/mp.svg"), "mp");
    countryEdit->addItem(QIcon(":/resources/countries/mq.svg"), "mq");
    countryEdit->addItem(QIcon(":/resources/countries/mr.svg"), "mr");
    countryEdit->addItem(QIcon(":/resources/countries/ms.svg"), "ms");
    countryEdit->addItem(QIcon(":/resources/countries/mt.svg"), "mt");
    countryEdit->addItem(QIcon(":/resources/countries/mu.svg"), "mu");
    countryEdit->addItem(QIcon(":/resources/countries/mv.svg"), "mv");
    countryEdit->addItem(QIcon(":/resources/countries/mw.svg"), "mw");
    countryEdit->addItem(QIcon(":/resources/countries/mx.svg"), "mx");
    countryEdit->addItem(QIcon(":/resources/countries/my.svg"), "my");
    countryEdit->addItem(QIcon(":/resources/countries/mz.svg"), "mz");
    countryEdit->addItem(QIcon(":/resources/countries/na.svg"), "na");
    countryEdit->addItem(QIcon(":/resources/countries/nc.svg"), "nc");
    countryEdit->addItem(QIcon(":/resources/countries/ne.svg"), "ne");
    countryEdit->addItem(QIcon(":/resources/countries/nf.svg"), "nf");
    countryEdit->addItem(QIcon(":/resources/countries/ng.svg"), "ng");
    countryEdit->addItem(QIcon(":/resources/countries/ni.svg"), "ni");
    countryEdit->addItem(QIcon(":/resources/countries/nl.svg"), "nl");
    countryEdit->addItem(QIcon(":/resources/countries/no.svg"), "no");
    countryEdit->addItem(QIcon(":/resources/countries/np.svg"), "np");
    countryEdit->addItem(QIcon(":/resources/countries/nr.svg"), "nr");
    countryEdit->addItem(QIcon(":/resources/countries/nu.svg"), "nu");
    countryEdit->addItem(QIcon(":/resources/countries/nz.svg"), "nz");
    countryEdit->addItem(QIcon(":/resources/countries/om.svg"), "om");
    countryEdit->addItem(QIcon(":/resources/countries/pa.svg"), "pa");
    countryEdit->addItem(QIcon(":/resources/countries/pe.svg"), "pe");
    countryEdit->addItem(QIcon(":/resources/countries/pf.svg"), "pf");
    countryEdit->addItem(QIcon(":/resources/countries/pg.svg"), "pg");
    countryEdit->addItem(QIcon(":/resources/countries/ph.svg"), "ph");
    countryEdit->addItem(QIcon(":/resources/countries/pk.svg"), "pk");
    countryEdit->addItem(QIcon(":/resources/countries/pl.svg"), "pl");
    countryEdit->addItem(QIcon(":/resources/countries/pm.svg"), "pm");
    countryEdit->addItem(QIcon(":/resources/countries/pn.svg"), "pn");
    countryEdit->addItem(QIcon(":/resources/countries/pr.svg"), "pr");
    countryEdit->addItem(QIcon(":/resources/countries/ps.svg"), "ps");
    countryEdit->addItem(QIcon(":/resources/countries/pt.svg"), "pt");
    countryEdit->addItem(QIcon(":/resources/countries/pw.svg"), "pw");
    countryEdit->addItem(QIcon(":/resources/countries/py.svg"), "py");
    countryEdit->addItem(QIcon(":/resources/countries/qa.svg"), "qa");
    countryEdit->addItem(QIcon(":/resources/countries/re.svg"), "re");
    countryEdit->addItem(QIcon(":/resources/countries/ro.svg"), "ro");
    countryEdit->addItem(QIcon(":/resources/countries/rs.svg"), "rs");
    countryEdit->addItem(QIcon(":/resources/countries/ru.svg"), "ru");
    countryEdit->addItem(QIcon(":/resources/countries/rw.svg"), "rw");
    countryEdit->addItem(QIcon(":/resources/countries/sa.svg"), "sa");
    countryEdit->addItem(QIcon(":/resources/countries/sb.svg"), "sb");
    countryEdit->addItem(QIcon(":/resources/countries/sc.svg"), "sc");
    countryEdit->addItem(QIcon(":/resources/countries/sd.svg"), "sd");
    countryEdit->addItem(QIcon(":/resources/countries/se.svg"), "se");
    countryEdit->addItem(QIcon(":/resources/countries/sg.svg"), "sg");
    countryEdit->addItem(QIcon(":/resources/countries/sh.svg"), "sh");
    countryEdit->addItem(QIcon(":/resources/countries/si.svg"), "si");
    countryEdit->addItem(QIcon(":/resources/countries/sj.svg"), "sj");
    countryEdit->addItem(QIcon(":/resources/countries/sk.svg"), "sk");
    countryEdit->addItem(QIcon(":/resources/countries/sl.svg"), "sl");
    countryEdit->addItem(QIcon(":/resources/countries/sm.svg"), "sm");
    countryEdit->addItem(QIcon(":/resources/countries/sn.svg"), "sn");
    countryEdit->addItem(QIcon(":/resources/countries/so.svg"), "so");
    countryEdit->addItem(QIcon(":/resources/countries/sr.svg"), "sr");
    countryEdit->addItem(QIcon(":/resources/countries/ss.svg"), "ss");
    countryEdit->addItem(QIcon(":/resources/countries/st.svg"), "st");
    countryEdit->addItem(QIcon(":/resources/countries/sv.svg"), "sv");
    countryEdit->addItem(QIcon(":/resources/countries/sx.svg"), "sx");
    countryEdit->addItem(QIcon(":/resources/countries/sy.svg"), "sy");
    countryEdit->addItem(QIcon(":/resources/countries/sz.svg"), "sz");
    countryEdit->addItem(QIcon(":/resources/countries/tc.svg"), "tc");
    countryEdit->addItem(QIcon(":/resources/countries/td.svg"), "td");
    countryEdit->addItem(QIcon(":/resources/countries/tf.svg"), "tf");
    countryEdit->addItem(QIcon(":/resources/countries/tg.svg"), "tg");
    countryEdit->addItem(QIcon(":/resources/countries/th.svg"), "th");
    countryEdit->addItem(QIcon(":/resources/countries/tj.svg"), "tj");
    countryEdit->addItem(QIcon(":/resources/countries/tk.svg"), "tk");
    countryEdit->addItem(QIcon(":/resources/countries/tl.svg"), "tl");
    countryEdit->addItem(QIcon(":/resources/countries/tm.svg"), "tm");
    countryEdit->addItem(QIcon(":/resources/countries/tn.svg"), "tn");
    countryEdit->addItem(QIcon(":/resources/countries/to.svg"), "to");
    countryEdit->addItem(QIcon(":/resources/countries/tr.svg"), "tr");
    countryEdit->addItem(QIcon(":/resources/countries/tt.svg"), "tt");
    countryEdit->addItem(QIcon(":/resources/countries/tv.svg"), "tv");
    countryEdit->addItem(QIcon(":/resources/countries/tw.svg"), "tw");
    countryEdit->addItem(QIcon(":/resources/countries/tz.svg"), "tz");
    countryEdit->addItem(QIcon(":/resources/countries/ua.svg"), "ua");
    countryEdit->addItem(QIcon(":/resources/countries/ug.svg"), "ug");
    countryEdit->addItem(QIcon(":/resources/countries/um.svg"), "um");
    countryEdit->addItem(QIcon(":/resources/countries/us.svg"), "us");
    countryEdit->addItem(QIcon(":/resources/countries/uy.svg"), "uy");
    countryEdit->addItem(QIcon(":/resources/countries/uz.svg"), "uz");
    countryEdit->addItem(QIcon(":/resources/countries/va.svg"), "va");
    countryEdit->addItem(QIcon(":/resources/countries/vc.svg"), "vc");
    countryEdit->addItem(QIcon(":/resources/countries/ve.svg"), "ve");
    countryEdit->addItem(QIcon(":/resources/countries/vg.svg"), "vg");
    countryEdit->addItem(QIcon(":/resources/countries/vi.svg"), "vi");
    countryEdit->addItem(QIcon(":/resources/countries/vn.svg"), "vn");
    countryEdit->addItem(QIcon(":/resources/countries/vu.svg"), "vu");
    countryEdit->addItem(QIcon(":/resources/countries/wf.svg"), "wf");
    countryEdit->addItem(QIcon(":/resources/countries/ws.svg"), "ws");
    countryEdit->addItem(QIcon(":/resources/countries/ye.svg"), "ye");
    countryEdit->addItem(QIcon(":/resources/countries/yt.svg"), "yt");
    countryEdit->addItem(QIcon(":/resources/countries/za.svg"), "za");
    countryEdit->addItem(QIcon(":/resources/countries/zm.svg"), "zm");
    countryEdit->addItem(QIcon(":/resources/countries/zw.svg"), "zw");
    countryEdit->setCurrentIndex(0);
    for(int i = 0; i < countryEdit->count(); ++i)
    {
        if(countryEdit->itemText(i) == country)
        {
            countryEdit->setCurrentIndex(i);
            break;
        }
    }

    realnameLabel = new QLabel(tr("Real name:"));
    realnameEdit = new QLineEdit();
    realnameLabel->setBuddy(realnameEdit);
    realnameEdit->setText(realName);
    
    QGridLayout *grid = new QGridLayout;
    grid->addWidget(emailLabel, 0, 0);
    grid->addWidget(emailEdit, 0, 1);
    grid->addWidget(genderLabel, 1, 0);
    grid->addWidget(genderEdit, 1, 1);
    grid->addWidget(countryLabel, 2, 0);
    grid->addWidget(countryEdit, 2, 1);
    grid->addWidget(realnameLabel, 3, 0);
    grid->addWidget(realnameEdit, 3, 1);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(actCancel()));
         
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Edit user informations"));
    setFixedHeight(sizeHint().height());
    setMinimumWidth(300);
}

void DlgEditUser::actOk()
{
    accept();
}

void DlgEditUser::actCancel()
{
    reject();
}
