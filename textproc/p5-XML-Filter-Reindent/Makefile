# $NetBSD: Makefile,v 1.23 2019/08/11 13:23:53 wiz Exp $

DISTNAME=	XML-Filter-Reindent-0.03
PKGNAME=	p5-${DISTNAME}
PKGREVISION=	13
CATEGORIES=	textproc perl5
MASTER_SITES=	${MASTER_SITE_PERL_CPAN:=../../authors/id/T/TJ/TJMATHER/}

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	https://metacpan.org/release/TJMATHER/XML-Filter-Reindent-0.03
COMMENT=	Reformats whitespace for pretty printing XML

CONFLICTS+=	p5-libxml-enno-[0-9]*

DEPENDS+=	p5-XML-Filter-DetectWS>=0.01:../../textproc/p5-XML-Filter-DetectWS

PERL5_PACKLIST=	auto/XML/Filter/Reindent/.packlist

.include "../../lang/perl5/module.mk"
.include "../../mk/bsd.pkg.mk"
