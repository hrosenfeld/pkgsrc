# $NetBSD: Makefile,v 1.2 2013/04/24 11:15:42 rodent Exp $
#

DISTNAME=	aspell-nl-0.50-2
PKGNAME=	${DISTNAME:S/-2/.2/1}
CATEGORIES=	textproc
MASTER_SITES=	${MASTER_SITE_GNU:=aspell/dict/nl/}
EXTRACT_SUFX=	.tar.bz2

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	http://aspell.net/
COMMENT=	Dutch language support for aspell

CONFLICTS+=	aspell-dutch-[0-9]*
HAS_CONFIGURE=	yes

.include "../../textproc/aspell/buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
